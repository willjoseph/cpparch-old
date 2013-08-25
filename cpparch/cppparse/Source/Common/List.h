
#ifndef INCLUDED_CPPPARSE_LIST_H
#define INCLUDED_CPPPARSE_LIST_H

#include <iterator>

#include "Allocator.h"

struct ListError
{
	ListError()
	{
	}
};

#define LIST_ASSERT(condition) if(!(condition)) { throw ListError(); }

template<typename T>
struct ListNode;
template<typename T>
struct ListNodeBase
{
	ListNode<T>* next;
};
template<typename T>
struct ListNode : ListNodeBase<T>
{
	T value;
	explicit ListNode(const T& value)
		: value(value)
	{
	}
};

template<typename T>
struct ListIterator
{
	typedef std::forward_iterator_tag iterator_category;
	typedef typename TypeTraits<T>::Value value_type;
	typedef ptrdiff_t difference_type;
	typedef T* pointer;
	typedef T& reference;

	typedef ListNode<value_type> Node;
	Node* p;
	explicit ListIterator(Node* p)
		: p(p)
	{
	}
	explicit ListIterator(const Node* p)
		: p(const_cast<Node*>(p))
	{
	}
	ListIterator(const ListIterator<value_type>& other) // allow initialisation from a mutable iterator
		: p(other.p)
	{
	}

	reference operator*() const
	{
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(!isDeallocated(p));
#endif
		return p->value;
	}
	pointer operator->() const
	{
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(!isDeallocated(p));
#endif
		return &p->value;
	}

	ListIterator& operator++()
	{
		p = p->next;
		return *this;
	}
	ListIterator operator++(int)
	{
		ListIterator tmp = *this;
		++*this;
		return tmp;
	}
};

template<typename T, typename Other>
inline bool operator==(ListIterator<T> left, ListIterator<Other> right)
{
	return &(*left) == &(*right);
}
template<typename T, typename Other>
inline bool operator!=(ListIterator<T> left, ListIterator<Other> right)
{
	return &(*left) != &(*right);
}

template<typename T, typename A>
struct List : protected A
{
	typedef ListNode<T> Node;
	ListNodeBase<T> head;
	Node* tail;
	typedef typename A::template rebind<Node>::other Allocator;

	A& getAllocator()
	{
		return *this;
	}
	const A& getAllocator() const
	{
		return *this;
	}

	List()
	{
		construct();
	}
	List(const A& allocator) :  A(allocator)
	{
		construct();
	}
	~List()
	{
		destroy();
	}
	void construct()
	{
		head.next = tail = static_cast<Node*>(&head);
	}
	void destroy()
	{
		for(Node* p = head.next; p != &head; )
		{
			Node* next = p->next;
			allocatorDelete(getAllocator(), p);
			p = next;
		}
	}

	List(const List& other)
		: A(other)
	{
		tail = static_cast<Node*>(&head);
		for(Node* p = other.head.next; p != &other.head; p = p->next)
		{
			tail->next = allocatorNew(getAllocator(), Node(p->value));
			tail = tail->next;
		}
		tail->next = static_cast<Node*>(&head);
	}
	List& operator=(List other)
	{
		other.swap(*this);
		return *this;
	}

	typedef ListIterator<T> iterator;
	typedef ListIterator<const T> const_iterator;

	iterator begin()
	{
		return iterator(head.next);
	}
	const_iterator begin() const
	{
		return const_iterator(head.next);
	}
	iterator end()
	{
		return iterator(static_cast<Node*>(&head));
	}
	const_iterator end() const
	{
		return const_iterator(static_cast<const Node*>(&head));
	}
	bool empty() const
	{
		return begin() == end();
	}
	T& front()
	{
		return head.next->value;
	}
	T& back()
	{
		return tail->value;
	}
	void clear()
	{
		destroy();
		construct();
	}
	
	void push_back(const T& value)
	{
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(empty() || !isDeallocated(head.next));
		ALLOCATOR_ASSERT(empty() || !isDeallocated(tail));
#endif
		Node* node = allocatorNew(getAllocator(), Node(value));
		node->next = static_cast<Node*>(&head);
		tail->next = node;
		tail = node;
	}
	void push_front(const T& value)
	{
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(empty() || !isDeallocated(head.next));
		ALLOCATOR_ASSERT(empty() || !isDeallocated(tail));
#endif
		Node* node = allocatorNew(getAllocator(), Node(value));
		if(empty())
		{
			tail = node;
		}
		node->next = head.next;
		head.next = node;
	}
	void pop_front()
	{
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(empty() || !isDeallocated(head.next));
		ALLOCATOR_ASSERT(empty() || !isDeallocated(tail));
#endif
		LIST_ASSERT(!empty());
		Node* p = head.next;
		head.next = p->next;
		allocatorDelete(getAllocator(), p);
	}
	void pop_back()
	{
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(empty() || !isDeallocated(head.next));
		ALLOCATOR_ASSERT(empty() || !isDeallocated(tail));
#endif
		LIST_ASSERT(!empty());
		Node* p = static_cast<Node*>(&head);
		for(; p->next != tail; )
		{
			p = p->next;
		}
		allocatorDelete(getAllocator(), tail);
		tail = p;
		tail->next = static_cast<Node*>(&head);
	}
	void erase(iterator first, iterator last)
	{
		LIST_ASSERT(first == begin());
		for(Node* p = head.next; p != last.p; )
		{
			Node* next = p->next;
			allocatorDelete(getAllocator(), p);
			p = next;
		}
		head.next = last.p;
	}
	void splice(iterator position, List& other)
	{
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(empty() || !isDeallocated(head.next));
		ALLOCATOR_ASSERT(empty() || !isDeallocated(tail));
		ALLOCATOR_ASSERT(empty() || !isDeallocated(other.head.next));
		ALLOCATOR_ASSERT(empty() || !isDeallocated(other.tail));
#endif
		//LIST_ASSERT(position == end());
		LIST_ASSERT(&other != this);
		// always splice at end for now
		if(!other.empty())
		{
			tail->next = other.head.next;
			tail = other.tail;
			tail->next = static_cast<Node*>(&head);
			other.construct();
		}
	}
	void swap(List& other)
	{
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(empty() || !isDeallocated(head.next));
		ALLOCATOR_ASSERT(empty() || !isDeallocated(tail));
		ALLOCATOR_ASSERT(empty() || !isDeallocated(other.head.next));
		ALLOCATOR_ASSERT(empty() || !isDeallocated(other.tail));
#endif
		std::swap(head, other.head);
		std::swap(tail, other.tail);
		if(head.next != &other.head)
		{
			tail->next = static_cast<Node*>(&head);
		}
		else
		{
			construct();
		}
		if(other.head.next != &head)
		{
			other.tail->next = static_cast<Node*>(&other.head);
		}
		else
		{
			other.construct();
		}
	}
};


template<typename T>
struct ListReferenceNode;
template<typename T>
struct ListReferenceNodeBase
{
	Reference< ListReferenceNode<T> > next;
};
template<typename T>
struct ListReferenceNode : ListReferenceNodeBase<T>
{
	T value;
	explicit ListReferenceNode(const T& value)
		: value(value)
	{
	}
};

template<typename T>
struct ListReferenceIterator
{
	typedef std::forward_iterator_tag iterator_category;
	typedef typename TypeTraits<T>::Value value_type;
	typedef ptrdiff_t difference_type;
	typedef T* pointer;
	typedef T& reference;

	typedef ListReferenceNode<value_type> Node;
	typedef Reference<Node> Pointer;
	Pointer p;
	explicit ListReferenceIterator(Pointer p)
		: p(p)
	{
	}
	ListReferenceIterator(const ListReferenceIterator<value_type>& other) // allow initialisation from a mutable iterator
		: p(other.p)
	{
	}

	reference operator*() const
	{
		return p->value;
	}
	pointer operator->() const
	{
		return &p->value;
	}

	ListReferenceIterator& operator++()
	{
		p = p->next;
		return *this;
	}
	ListReferenceIterator operator++(int)
	{
		ListReferenceIterator tmp = *this;
		++*this;
		return tmp;
	}
};

template<typename T, typename Other>
inline bool operator==(ListReferenceIterator<T> left, ListReferenceIterator<Other> right)
{
	return &(*left) == &(*right);
}
template<typename T, typename Other>
inline bool operator!=(ListReferenceIterator<T> left, ListReferenceIterator<Other> right)
{
	return &(*left) != &(*right);
}


template<typename T, typename A>
struct ListReference : A
{
	typedef ListReferenceNode<T> Node;
	typedef Reference<Node> Pointer;
	ListReferenceNodeBase<T> head;
	Pointer tail;
	typedef typename A::template rebind<Node>::other Allocator;

	A& getAllocator()
	{
		return *this;
	}
	const A& getAllocator() const
	{
		return *this;
	}

	ListReference()
	{
		construct();
	}
	ListReference(const A& allocator) :  A(allocator)
	{
		construct();
	}
	ListReference& operator=(const ListReference& other)
	{
		head = other.head;
		tail = other.tail;
		return *this;
	}
	void construct()
	{
		head.next = tail = 0;
	}

	typedef ListReferenceIterator<T> iterator;
	typedef ListReferenceIterator<const T> const_iterator;

	iterator begin()
	{
		return iterator(head.next);
	}
	const_iterator begin() const
	{
		return const_iterator(head.next);
	}
	iterator end()
	{
		return iterator(Pointer(0));
	}
	const_iterator end() const
	{
		return const_iterator(Pointer(0));
	}
	bool empty() const
	{
		return begin() == end();
	}
	T& front()
	{
		return head.next->value;
	}
	T& back()
	{
		return tail->value;
	}
	void clear()
	{
		construct();
	}

	void push_back(const T& value)
	{
		Pointer node = allocatorNew(getAllocator(), makeReferenceCounted(Node(value)));
		node->next = 0;
		if(empty())
		{
			head.next = node;
		}
		else
		{
			tail->next = node;
		}
		tail = node;
	}
	void push_front(const T& value)
	{
		Pointer node = allocatorNew(getAllocator(), makeReferenceCounted(Node(value)));
		node->next = head.next;
		if(empty())
		{
			tail = node;
		}
		head.next = node;
	}
	void splice(iterator position, ListReference& other)
	{
		LIST_ASSERT(position == begin());
		LIST_ASSERT(&other != this);
		// always splice at begin for now
		if(!other.empty())
		{
			if(empty())
			{
				tail = other.tail;
			}
			else
			{
				other.tail->next = head.next;
			}
			head = other.head;
			other.construct();
		}
	}
	void swap(ListReference& other)
	{
		head.next.swap(other.head.next);
		tail.swap(other.tail);
	}
};
#endif


