
#ifndef INCLUDED_CPPPARSE_LIST_H
#define INCLUDED_CPPPARSE_LIST_H

struct ListNodeBase
{
	ListNodeBase* next;
};
template<typename T>
struct ListNode : ListNodeBase
{
	T value;
	ListNode(const T& value)
		: value(value)
	{
	}
};

template<typename T>
struct ListIterator
{
	typedef ListNode<T> Node;
	const ListNodeBase* p;
	ListIterator(const ListNodeBase* p) : p(p)
	{
	}

	const T& operator*() const
	{
		return static_cast<const Node*>(p)->value;
	}
	const T* operator->() const
	{
		return &static_cast<const Node*>(p)->value;
	}

	ListIterator<T>& operator++()
	{
		p = p->next;
		return *this;
	}
	ListIterator<T> operator++(int)
	{
		ListIterator<T> tmp = *this;
		++*this;
		return tmp;
	}
};

template<typename T>
inline bool operator==(const ListIterator<T>& left, const ListIterator<T>& right)
{
	return left.p == right.p;
}
template<typename T>
inline bool operator!=(const ListIterator<T>& left, const ListIterator<T>& right)
{
	return left.p != right.p;
}

template<typename T, typename A>
struct List : private A
{
	typedef ListNode<T> Node;
	ListNodeBase head;
	ListNodeBase* tail;
	typedef typename A::template rebind<Node>::other Allocator;

	List()
	{
		init_raw();
	}
	List(const A& allocator) :  A(allocator)
	{
		init_raw();
	}
	~List()
	{
		for(ListNodeBase* p = head.next; p != &head; )
		{
			ListNodeBase* next = p->next;
			allocatorDelete(*this, static_cast<Node*>(p));
			p = next;
		}
	}
	void init_raw()
	{
		head.next = tail = &head;
	}
	void copy_raw(const List& other)
	{
		if(other.empty())
		{
			new(this) List();
		}
		else
		{
			head = other.head;
			tail = other.tail;
			tail->next = &head;
		}
	}
private:
	List(const List& other);
	List& operator=(const List& other);
public:

	typedef ListIterator<T> const_iterator;

	const_iterator begin() const
	{
		return const_iterator(head.next);
	}
	const_iterator end() const
	{
		return const_iterator(&head);
	}
	bool empty() const
	{
		return head.next == &head;
	}
	T& back()
	{
		return static_cast<Node*>(tail)->value;
	}
	
	void push_back(const T& value)
	{
		Node* node = allocatorNew(*this, value);
		node->next = &head;
		tail->next = node;
		tail = node;
	}
	void splice(const_iterator position, List& other)
	{
		// always splice at end for now
		if(!other.empty())
		{
			tail->next = other.head.next;
			tail = other.tail;
			tail->next = &head;
		}
	}
	void swap(List& other)
	{
		std::swap(head, other.head);
		std::swap(tail, other.tail);
		if(head.next != &other.head)
		{
			tail->next = &head;
		}
		else
		{
			init_raw();
		}
		if(other.head.next != &head)
		{
			other.tail->next = &other.head;
		}
		else
		{
			other.init_raw();
		}
	}
};

#endif


