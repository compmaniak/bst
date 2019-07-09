#include <memory>
#include <functional>
#include <iostream>
#include <iomanip>
#include <vector>
#include <limits>
#include <cassert>

namespace detail
{

template<class T>
struct node
{
    T v;
    node *p = nullptr; // parent
    std::unique_ptr<node> l, r; // left, right child
};

template<class T, class Vis>
void visit(node<T> const *root, Vis&& vis, size_t h = 1)
{
    if (root) {
        visit(root->l.get(), vis, h + 1);
        vis(h, root->v);
        visit(root->r.get(), vis, h + 1);
    }
}

template<class T>
bool is_balanced(node<T> const *root, size_t& h)
{
    size_t lh = 0, rh = 0;
    if (root) {
        if (!is_balanced(root->l.get(), lh))
            return false;
        if (!is_balanced(root->r.get(), rh))
            return false;
        if (lh > rh)
            std::swap(lh, rh);
        h = rh + 1;
    } else {
        h = 0;
    }
    return rh - lh <= 1;
}

} // namespace detail

template<class T, class Compare = std::less<T>>
class binary_tree
{
    using node = detail::node<T>;
    using node_ptr = std::unique_ptr<node>;

public:
    bool find(T const& v) const { return find_at(root_.get(), v) != nullptr; }
    
    bool insert(T const& v) { return insert_at(root_, T(v)); }
    bool insert(T&& v) { return insert_at(root_, std::move(v)); }

    bool remove(T const& v)
    {
        node *n = find_at(root_.get(), v);
        if (n == nullptr)
            return false;
        remove(n);
        return true;
    }

    template<class Vis>
    void visit(Vis&& vis) const { detail::visit(root_.get(), vis); }

    bool is_balanced() const
    {
        size_t h;
        return detail::is_balanced(root_.get(), h);
    }

private:
    bool insert_at(node_ptr& np, T&& v, node *p = nullptr)
    {
        if (!np) {
            np.reset(new node());
            np->v = std::move(v);
            np->p = p;
            return true;
        } else if (comp_(v, np->v)) {
            return insert_at(np->l, std::move(v), np.get());
        } else if (comp_(np->v, v)) {
            return insert_at(np->r, std::move(v), np.get());
        } else {
            return false;
        }
    }

    node* find_at(node *root, T const& v) const
    {
        if (!root)
            return nullptr;
        if (comp_(v, root->v))
            return find_at(root->l.get(), v);
        if (comp_(root->v, v))
            return find_at(root->r.get(), v);
        return root;
    }

    void remove(node *n)
    {
        if (n->l && n->r) { // has both children
            node *m = n->r.get();
            while (m->l)
                m = m->l.get();
            n->v = std::move(m->v);
            remove(m);
        } else if (n->l) { // has left child only
            n->v = std::move(n->l->v);
            n->r = std::move(n->l->r);
            n->l = std::move(n->l->l);
            if (n->l) n->l->p = n;
            if (n->r) n->r->p = n;
        } else if (n->r) { // has right child only
            n->v = std::move(n->r->v);
            n->l = std::move(n->r->l);
            n->r = std::move(n->r->r);
            if (n->l) n->l->p = n;
            if (n->r) n->r->p = n;
        } else if (!n->p) { // has no parent so it must be the root
            assert(root_.get() == n);
            root_.reset();
        } else if (n->p->l.get() == n) {
            n->p->l.reset();
        } else {
            n->p->r.reset();
        }
    }

    node_ptr root_;
    Compare comp_;
};

int main()
{
    std::ios_base::sync_with_stdio(false);

    binary_tree<long> t;
    t.insert(6);
    t.insert(8);
    t.insert(4);
    t.insert(10);

    std::cout << "USAGE: +|-|? <long>\n\n";

    std::vector<std::pair<size_t, long>> items;
    for (;;) {
        size_t height = 0;
        items.clear();
        t.visit([&height, &items](size_t h, long v) {
            height = std::max(height, h);
            items.push_back({h, v});
        });

        std::string lv, rv;
        if (!items.empty()) {
            lv = std::to_string(items.front().second);
            rv = std::to_string(items.back().second);
        }
        size_t w = std::max(lv.size(), rv.size()) + 1;

        std::cout << "items:";
        for (auto& x: items)
            std::cout << ' ' << x.second;
        std::cout << '\n';
        std::cout << "height: " << height << '\n';
        std::cout << "balance: " << std::boolalpha << t.is_balanced() << '\n';
        for (auto it = items.rbegin(); it != items.rend(); ++it) {
            size_t h = (it->first - 1) * w;
            for (; h > 0; --h)
                std::cout.put(' ');
            std::cout << std::left << std::setw(w) << it->second << '\n';
        }
        std::cout << std::flush;

        for (bool stop = false; !stop; ) {
            char cmd;
            long val;
            std::cout << "> ";
            std::cin >> cmd >> val;
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "> ";
            if (cmd == '+')
                std::cout << (stop = t.insert(val));
            else if (cmd == '-')
                std::cout << (stop = t.remove(val));
            else if (cmd == '?')
                std::cout << t.find(val);
            else
                std::cout << "unknown command";
            std::cout << std::endl;
        }
    }
}
