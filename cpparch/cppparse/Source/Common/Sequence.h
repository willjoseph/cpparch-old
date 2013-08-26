
#ifndef INCLUDED_CPPPARSE_COMMON_SEQUENCE_H
#define INCLUDED_CPPPARSE_COMMON_SEQUENCE_H

#include "Allocator.h"

// ----------------------------------------------------------------------------
// sequence

template<typename Visitor>
struct SequenceNode
{
	Reference<SequenceNode> next;

	virtual ~SequenceNode()
	{
	}
	virtual void accept(Visitor& visitor) const = 0;
#if 0
	virtual bool operator==(const SequenceNode& other) const = 0;
#endif

	const SequenceNode* get() const
	{
		return next.get();
	}
};

template<typename Visitor>
struct SequenceNodeEmpty : SequenceNode<Visitor>
{
	virtual void accept(Visitor& visitor) const
	{
		throw AllocatorError();
	}
#if 0
	virtual bool operator==(const SequenceNode<Visitor>& other) const
	{
		throw AllocatorError();
	}
#endif
	virtual bool operator<(const SequenceNode<Visitor>& other) const
	{
		throw AllocatorError();
	}
};

template<typename T, typename Visitor>
struct SequenceNodeGeneric : Reference< SequenceNode<Visitor> >::Value
{
	T value;
	SequenceNodeGeneric(const T& value)
		: value(value)
	{
	}
	void accept(Visitor& visitor) const
	{
		visitor.visit(value);
	}
#if 0
	bool operator==(const SequenceNode<Visitor>& other) const
	{
		return typeid(*this) == typeid(other)
			&& value == static_cast<const SequenceNodeGeneric*>(&other)->value;
	}
#endif
#if 0
	bool operator<(const SequenceNode<Visitor>& other) const
	{
		return (typeid(*this).before(typeid(other)) ||
			!(typeid(other).before(typeid(*this))) && value < static_cast<const SequenceNodeGeneric*>(&other)->value);
	}
#endif
};

template<typename A, typename Visitor>
struct Sequence : A
{
	typedef SequenceNode<Visitor> Node;
	typedef Reference<Node> Pointer;
	SequenceNodeEmpty<Visitor> head;

	A& getAllocator()
	{
		return *this;
	}
	const A& getAllocator() const
	{
		return *this;
	}

	Sequence()
	{
		construct();
	}
	Sequence(const A& allocator)
		:  A(allocator)
	{
		construct();
	}
	Sequence& operator=(Sequence other)
	{
		head = other.head;
		return *this;
	}
	void construct()
	{
		head.next = 0;
	}

	bool empty() const
	{
		return head.next == Pointer(0);
	}
	void clear()
	{
		construct();
	}

	template<typename T>
	void push_front(const T& value)
	{
		Pointer node = allocatorNew(getAllocator(), SequenceNodeGeneric<T, Visitor>(value));
		node->next = head.next;
		head.next = node;
	}
	template<typename T>
	void push_back(const T& value)
	{
		Pointer node = allocatorNew(getAllocator(), SequenceNodeGeneric<T, Visitor>(value));
		if(empty())
		{
			node->next = head.next;
			head.next = node;
		}
		else
		{
			Pointer last = head.next;
			for(Pointer next = last->next; next.get() != 0; next = next->next)
			{
				last = next;
			}
			node->next = 0;
			last->next = node;
		}
	}
	void pop_front()
	{
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(head.next.p->count == 1);
#endif
		ALLOCATOR_ASSERT(!empty());
		Pointer node = head.next;
		head.next = node->next;
		allocatorDelete(getAllocator(), node.get());
	}

	void swap(Sequence& other)
	{
		head.next.swap(other.head.next);
	}
	void reverse()
	{
#ifdef ALLOCATOR_DEBUG
		ALLOCATOR_ASSERT(head.next.p->count == 1);
#endif
		Pointer root = head.next;
		head.next = 0;
		while(root != 0)
		{
			Pointer next = root->next;
			root->next = head.next;
			head.next = root;
			root = next;
		}
	}

	const Node* get() const
	{
		return head.next.get();
	}

	void accept(Visitor& visitor) const
	{
		for(const Node* node = get(); node != 0; node = node->get())
		{
			node->accept(visitor);
		}
	}
};

template<typename Visitor>
const SequenceNode<Visitor>* findLast(const SequenceNode<Visitor>* node)
{
	ALLOCATOR_ASSERT(node != 0);
	const SequenceNode<Visitor>* next = node->get();
	if(next == 0)
	{
		return node;
	}
	return findLast(next);
}



#endif


