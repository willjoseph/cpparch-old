
#ifndef INCLUDED_CPPPARSE_LIST_H
#define INCLUDED_CPPPARSE_LIST_H

#include <iterator>

#include "allocator.h"

template<typename T>
struct TypeTraits
{
	typedef T Value;
};

template<typename T>
struct TypeTraits<const T>
{
	typedef T Value;
};

struct ListError
{
	ListError()
	{
	}
};

#define LIST_ASSERT(condition) if(!(condition)) { throw ListError(); }

struct ListNodeBase
{
	ListNodeBase* next;
};
template<typename T>
struct ListNode : ListNodeBase
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
	ListNodeBase* p;
	ListIterator(ListNodeBase* p)
		: p(p)
	{
	}
	ListIterator(const ListNodeBase* p)
		: p(const_cast<ListNodeBase*>(p))
	{
	}
	ListIterator(const ListIterator<value_type>& other) // allow initialisation from a non-const iterator
		: p(other.p)
	{
	}

	reference operator*() const
	{
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(!isDeallocated(p));
#endif
		return static_cast<Node*>(p)->value;
	}
	pointer operator->() const
	{
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(!isDeallocated(p));
#endif
		return &static_cast<Node*>(p)->value;
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
struct List : private A
{
	typedef ListNode<T> Node;
	ListNodeBase head;
	ListNodeBase* tail;
	typedef typename A::template rebind<Node>::other Allocator;

	A& getAllocator()
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
		head.next = tail = &head;
	}
	void destroy()
	{
		for(ListNodeBase* p = head.next; p != &head; )
		{
			ListNodeBase* next = p->next;
			allocatorDelete(getAllocator(), static_cast<Node*>(p));
			p = next;
		}
	}

	List(const List& other)
		: A(other)
	{
		tail = &head;
		for(ListNodeBase* p = other.head.next; p != &other.head; p = p->next)
		{
			tail->next = allocatorNew(getAllocator(), Node(static_cast<Node*>(p)->value));
			tail = tail->next;
		}
		tail->next = &head;
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
		return iterator(&head);
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
	void clear()
	{
		destroy();
		construct();
	}
	
	void push_back(const T& value)
	{
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(!isDeallocated(head.next));
		ALLOCATOR_ASSERT(!isDeallocated(tail));
#endif
		Node* node = allocatorNew(getAllocator(), Node(value));
		node->next = &head;
		tail->next = node;
		tail = node;
	}
	void splice(iterator position, List& other)
	{
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(!isDeallocated(head.next));
		ALLOCATOR_ASSERT(!isDeallocated(tail));
		ALLOCATOR_ASSERT(!isDeallocated(other.head.next));
		ALLOCATOR_ASSERT(!isDeallocated(other.tail));
#endif
		LIST_ASSERT(position == end());
		LIST_ASSERT(&other != this);
		// always splice at end for now
		if(!other.empty())
		{
			tail->next = other.head.next;
			tail = other.tail;
			tail->next = &head;
			other.construct();
		}
	}
	void swap(List& other)
	{
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(!isDeallocated(head.next));
		ALLOCATOR_ASSERT(!isDeallocated(tail));
		ALLOCATOR_ASSERT(!isDeallocated(other.head.next));
		ALLOCATOR_ASSERT(!isDeallocated(other.tail));
#endif
		std::swap(head, other.head);
		std::swap(tail, other.tail);
		if(head.next != &other.head)
		{
			tail->next = &head;
		}
		else
		{
			construct();
		}
		if(other.head.next != &head)
		{
			other.tail->next = &other.head;
		}
		else
		{
			other.construct();
		}
	}
};

#endif


