//
// Created by xc5 on 2024/11/4.
//

#ifndef OCC_COM_SLIST_H
#define OCC_COM_SLIST_H

#include "basic.h"
#include "consts.h"
#include "ir.h"

  class SLIST_NODE {
  friend class SLIST;
  friend class SLIST_ITER;
private:
  SLIST_NODE *_next;   // point to the next node in the singly linked list

  SLIST_NODE& operator= (const SLIST_NODE& sl);
  SLIST_NODE(const SLIST_NODE&);

protected:
  SLIST_NODE(void)                       { _next = NULL; }
  ~SLIST_NODE(void)                      {}

  void        Insert_After(SLIST_NODE *nd)  { nd->_next = _next; _next = nd; }
  SLIST_NODE *Insert_Before(SLIST_NODE *nd) { nd->_next = this; return nd; }
  SLIST_NODE *Remove(SLIST_NODE *prev);
  void        Set_Next(SLIST_NODE *n)       { _next = n; }
  INT32       Len(void) const;
  INT         Pos(SLIST_NODE *) const;
public:
  SLIST_NODE *Next(void) const              { return _next;}
};



class SLIST {
private:
  SLIST_NODE *_head;
  SLIST_NODE *_tail;

  SLIST& operator= (const SLIST& sl);
  SLIST(const SLIST&);

protected:
  SLIST(void)                  { _head = _tail = NULL; }
public:
  SLIST(SLIST_NODE *list);
  ~SLIST(void)                 {}

  void Set_Head(SLIST_NODE *h)      { _head = h; }
  void Set_Tail(SLIST_NODE *t)      { _tail = t; }

  void        Init(SLIST_NODE *list);   // same as constructor
  void        Init_Head(SLIST_NODE *list)      { _head = list; _tail = NULL; }
  void        Clear(void)                      { _head = _tail = NULL; }

  BOOL        Append( SLIST_NODE *nd, SLIST_NODE *od);
  BOOL        Prepend( SLIST_NODE *nd, SLIST_NODE *od );
  void        Append_List(SLIST *new_list);
  void        Prepend_List(SLIST *new_list);
  SLIST_NODE *Remove_Headnode(void);
  SLIST_NODE *Remove(SLIST_NODE *prev, SLIST_NODE *cur);
  void        Remove_node(SLIST_NODE *slist_node);

  SLIST_NODE            *Head(void)            { return _head; }
  const SLIST_NODE      *Head(void) const      { return _head; }
  SLIST_NODE            *Tail(void)            { return _tail; }
  const SLIST_NODE      *Tail(void) const      { return _tail; }
  BOOL        Is_Empty(void) const             { return _head == NULL; }
  INT32       Len(void) const;
  INT         Pos(SLIST_NODE *nd) const        { return _head->Pos(nd); }
  void Append( SLIST_NODE *nd ) {
    if (nd == NULL) return;
    if (_head == NULL)
      _head = _tail = nd;
    else {
      _tail->Insert_After(nd);
      _tail = _tail->Next();
    }
  }

  void Prepend( SLIST_NODE *nd ) {
    if (nd == NULL) return;
    // insert nd to beginning of list
    if (_head == NULL)
      _head = _tail = nd;
    else {
      _head = _head->Insert_Before(nd);
    }
  }
};

class SLIST_ITER {
private:
  SLIST_NODE  *_head;
  SLIST_NODE  *_cur;
  mINT32       _len;
  mINT32       _idx;          // simulate indexing, default to -1

  SLIST_ITER& operator= (const SLIST_ITER& sl);
  SLIST_ITER(const SLIST_ITER&);

protected:
  SLIST_ITER(void) { _head = NULL; _cur = NULL; _len = -1; _idx = -1;}
  void Set_Cur(SLIST_NODE* cur) { _cur = cur;}
  void Set_Idx(mINT16 idx) {_idx = idx;}

public:
  SLIST_ITER(SLIST_NODE *nd)
  { _head = nd; _cur = _head; _len = -1; _idx = -1; }
  SLIST_ITER(SLIST *sl)
  { _head=sl->Head(); _cur=_head; _len=-1; _idx=-1; }
  ~SLIST_ITER(void) {}

  void         Init(SLIST_NODE *nd) { _head = nd; _cur = _head;}
  void         Init(SLIST *sl) {
    _head = (sl ? sl->Head() : NULL);
  }
  void         Clear(void) { _head = NULL; _cur = NULL; _len = -1; _idx = -1;}
  void         Set(SLIST_NODE *nd)  {_cur = nd; _idx = -1;}
  SLIST_NODE  *First(void) {  // get the first element, and reset the pointer
    if (this == NULL)
      return NULL;
    if (_head)
      _cur = _head;
    else
      _cur = NULL;
    _idx = 0;
    return _cur;
  }
  SLIST_NODE  *Next(void) {   // get the next element, and bump the pointer.
    if (this == NULL)
      return NULL;
    if (_cur != NULL) {
      _cur = _cur->Next();
      _idx++;
    }
    return _cur;
  }
  SLIST_NODE  *Nth(INT n);// get the nth element

  SLIST_NODE  *Peek_Next(void) const {return _cur->Next();}
  SLIST_NODE  *Head(void) const         {return _head;}
  SLIST_NODE  *Cur(void) const          {return _cur;}
  mINT32       Idx(void) const          {return _idx;}
  mINT32       Len(void);               // get the length of the list
  BOOL         Is_Empty(void) const     { return _cur == NULL; }
};

// DECLARE_SLIST_NODE_CLASS: a macro to define the body of a derived
//                           class "NAME_LIST" of SLIST_NODE
//                   Example:
//                           class NAME_LIST : public SLIST_NODE {
//                           DECLARE_SLIST_NODE_CLASS( NAME_LIST )
//                           ~NAME_LIST(void);
//                           };
//                   Note:   Create your own destructor!

#define DECLARE_SLIST_NODE_CLASS( NAME_NODE )                         \
                                                                      \
public:                                                               \
  NAME_NODE *Next(void) {                                             \
    return (NAME_NODE *) SLIST_NODE::Next();                          \
  }                                                                   \
  const NAME_NODE *Next(void) const {                                 \
    return (NAME_NODE *) SLIST_NODE::Next();                          \
  }                                                                   \
  void Insert_After(NAME_NODE *nd) {                                  \
    SLIST_NODE::Insert_After(nd);                                     \
  }                                                                   \
  void Insert_Before(NAME_NODE *nd) {                                 \
    SLIST_NODE::Insert_Before(nd);                                    \
  }                                                                   \
  NAME_NODE *Remove(NAME_NODE *prev) {                                \
    return (NAME_NODE*) SLIST_NODE::Remove(prev);                     \
  }                                                                   \
  void Set_Next(NAME_NODE *nd) {                                      \
  }                                                                   \
  void Set_Next(NAME_NODE *nd) {                                      \
    SLIST_NODE::Set_Next(nd);                                         \
  }                                                                   \
  INT32  Len(void) const    { return SLIST_NODE::Len(); }             \
  INT    Pos(NAME_NODE *od) { return SLIST_NODE::Pos(od); }           \
                                                                      \
                                                                      \
// Some Declaration here.
#define DECLARE_SLIST_CLASS( NAME_LIST, NAME_NODE)                      \
public:                                                                 \
  typedef NAME_NODE CONTAINER_NODE;                                     \
  NAME_LIST(NAME_NODE *nd) { SLIST::Init(nd); }                         \
  NAME_LIST() : SLIST() {}                                              \
  void Append(NAME_NODE *nd) { SLIST::Append(nd); }                     \
  BOOL Append(NAME_NODE *nd, NAME_NODE *od) {                           \
    return SLIST::Append(nd, od);                                       \
  }                                                                     \
  void Prepend(NAME_NODE *nd) { SLIST::Prepend(nd); }                   \
  BOOL Prepend(NAME_NODE *nd, NAME_NODE *od) {                          \
    return SLIST::Prepend(nd, od);                                      \
  }                                                                     \
  void Append_List(NAME_LIST *nl) { SLIST::Append_List(nl); }           \
  void Prepend_List(NAME_LIST *nl) { SLIST::Prepend_List(nl); }         \
  NAME_NODE *Remove_Headnode(void) {                                    \
    return (NAME_NODE*) SLIST::Remove_Headnode();                       \
  }                                                                     \
  NAME_NODE *Remove(NAME_NODE *prev, NAME_NODE *cur) {                  \
    return (NAME_NODE*) SLIST::Remove(prev, cur);                       \
  }                                                                     \
  NAME_NODE *Head(void) { return (NAME_NODE *) SLIST::Head(); }         \
  const NAME_NODE *Head(void) const                                     \
                { return (const NAME_NODE *) SLIST::Head(); }           \
  NAME_NODE *Tail(void) { return (NAME_NODE *) SLIST::Tail(); }         \
  const NAME_NODE *Tail(void) const                                     \
                { return (const NAME_NODE *) SLIST::Tail(); }           \
  BOOL       Is_Empty(void) const { return SLIST::Is_Empty(); }         \
  INT32      Len(void) const { return SLIST::Len(); }                   \
  INT        Pos(NAME_NODE *od) { return SLIST::Pos(od); }              \

// Extra

#define DECLARE_SLIST_ITER_CLASS( NAME_ITER, NAME_NODE, NAME_LIST)      \
public:                                                                 \
  NAME_ITER(NAME_NODE *nd) { SLIST_ITER::Init(nd); }                    \
  NAME_ITER(NAME_LIST *nl) { SLIST_ITER::Init(nl); }                    \
  NAME_ITER(void)          { SLIST_ITER::Init();   }                    \
  void Init(NAME_NODE *nd) { SLIST_ITER::Init(nd); }                    \
  void Init(NAME_LIST *nl) { SLIST_ITER::Init(nl); }                    \
  void Set(NAME_NODE *nd) { SLIST_ITER::Set(nd); }                      \
  NAME_NODE *First(void) { return (NAME_NODE *) SLIST_ITER::First(); }  \
  NAME_NODE *Next(void) { return (NAME_NODE *) SLIST_ITER::Next(); }    \
  NAME_NODE *Nth(INT n) { return (NAME_NODE *) SLIST_ITER::Nth(n); }    \
  NAME_NODE *Peek_Next(void) { return (NAME_NODE *) SLIST_ITER::Peek_Next(); }\
  NAME_NODE *Head(void) { return (NAME_NODE *) SLIST_ITER::Head(); }    \
  NAME_NODE *Cur(void) { return (NAME_NODE *) SLIST_ITER::Cur(); }      \
  BOOL Is_Empty(void) { return SLIST_ITER::Is_Empty(); }                \

#define DECLARE_SLIST_CONST_ITER_CLASS( NAME_ITER, NAME_NODE, NAME_LIST) \
public:                                                                 \
  NAME_ITER(const NAME_NODE *nd) { SLIST_ITER::Init((NAME_NODE*)nd); } \
  NAME_ITER(const NAME_LIST *nl) { SLIST_ITER::Init((NAME_LIST*)nl); } \
  NAME_ITER(void)          { SLIST_ITER::Init();   }                    \
  void Init(NAME_NODE *nd) { SLIST_ITER::Init(nd); }                    \
  void Init(NAME_LIST *nl) { SLIST_ITER::Init(nl); }                    \
  void Set(NAME_NODE *nd) { SLIST_ITER::Set(nd); }                      \
  const NAME_NODE *First(void) { return (NAME_NODE *) SLIST_ITER::First(); } \
  const NAME_NODE *Next(void) { return (NAME_NODE *) SLIST_ITER::Next(); }   \
  const NAME_NODE *Nth(INT n) { return (NAME_NODE *) SLIST_ITER::Nth(n); }   \
  const NAME_NODE *Peek_Next(void) { return (NAME_NODE *) SLIST_ITER::Peek_Next(); }   \
  const NAME_NODE *Head(void) { return (NAME_NODE *) SLIST_ITER::Head(); }   \
  const NAME_NODE *Cur(void) { return (NAME_NODE *) SLIST_ITER::Cur(); }     \
  BOOL Is_Empty(void) { return SLIST_ITER::Is_Empty(); }                 \

//
//         void CHAIN_NODE::Set_Next(CHAIN_NODE *nd)
//
//             Sets the "next" field of "this" node.
//
//         void CHAIN_NODE::Set_Prev(CHAIN_NODE *nd)
//
//             Sets the "prev" field of "this" node.
//

class CHAIN_NODE {
  friend class CHAIN;
  friend class CHAIN_ITER;
private:
  CHAIN_NODE *_next;   // point to the next node in the doubly linked list
  CHAIN_NODE *_prev;   // point to the prev node in the doubly linked list

  CHAIN_NODE& operator= (const CHAIN_NODE& sl);
  CHAIN_NODE(const CHAIN_NODE&);

protected:
  CHAIN_NODE(void)                       { _next = _prev = NULL; }
  ~CHAIN_NODE(void)                      {}

  CHAIN_NODE *Insert_After(CHAIN_NODE *nd);
  CHAIN_NODE *Insert_Before(CHAIN_NODE *nd);
  CHAIN_NODE *Remove(void);

  CHAIN_NODE *Next(void)                    { return _next;}
  const CHAIN_NODE *Next(void) const        { return _next;}
  CHAIN_NODE *Prev(void)                    { return _prev;}
  const CHAIN_NODE *Prev(void) const        { return _prev;}
  void        Set_Next(CHAIN_NODE *n)       { _next = n; }
  void        Set_Prev(CHAIN_NODE *n)       { _prev = n; }
};

// CHAIN     : the container class is typically used to construct a
//             doubly linked list that is based on the base class,
//             CHAIN_NODE.  It maintains the head and the tail of the
//             doubly linked list for fast update.
//
//       Exported Functions:
//
//         CHAIN::CHAIN(void)
//
//             Construct a doubly linked list container, initialize head and
//             tail to NULL
//
//         CHAIN::CHAIN(CHAIN_NODE *nd)
//
//             Construct a doubly linked list container, initialize head and
//             tail to nd.
//
//         CHAIN::~CHAIN(void)
//
//             Destruct a doubly linked list container, reset head and tail
//             to NULL.
//
//         void CHAIN::Init(CHAIN_NODE *node)
//
//             Initializes head and tail with node
//

class CHAIN {
private:
  CHAIN_NODE *_head;
  CHAIN_NODE *_tail;

  CHAIN& operator= (const CHAIN& sl);
  CHAIN(const CHAIN&);

protected:
  CHAIN(void)                  { _head = _tail = NULL; }

public:
  CHAIN(CHAIN_NODE *nd)        { _head = _tail = nd; }
  ~CHAIN(void)                 {}

  // same as constructor
  void        Init(void)                { _head = _tail = NULL; }
  void        Init(CHAIN_NODE *nd)      { _head = _tail = nd; }
  void        Init(CHAIN *list)
  { _head = list->Head(); _tail = list->Tail();}
  void        Clear(void)               { _head = _tail = NULL; }

  void        Append( CHAIN_NODE *nd );
  void        Prepend( CHAIN_NODE *nd );
  void        Insert_After(CHAIN_NODE *nd, CHAIN_NODE *after_nd);
  void        Insert_Before(CHAIN_NODE *nd, CHAIN_NODE *before_nd);
  BOOL        Is_Member(CHAIN_NODE *nd) const;

  void        Append_List(CHAIN *new_list);
  void        Prepend_List(CHAIN *new_list);
  void        Remove(CHAIN_NODE *);
  CHAIN_NODE *Remove_Head(void);
  CHAIN_NODE *Remove_Tail(void);

  CHAIN_NODE       *Head(void)                 { return _head; }
  const CHAIN_NODE *Head(void) const           { return _head; }
  CHAIN_NODE       *Tail(void)                       { return _tail; }
  const CHAIN_NODE *Tail(void) const                 { return _tail; }
};

// CHAIN_ITER: the iterator class is typically used to iterate through
//             a doubly linked list that is based on the base class
//             CHAIN_NODE, to perform certain operation.  It does not
//             change the list content.
//
//       Exported Functions:
//
//         CHAIN_ITER::CHAIN_ITER(void)
//
//             Construct a doubly linked list iter.  Initialize list/cur to
//             NULL and len/idx to -1.
//
//         CHAIN_ITER::Set_Cur(CHAIN_NODE *cur)
//
//             Sets the _cur node to the specified 'cur' node.  This allow
//             caller to visit part of the chain starting from 'cur' node.
//             NOTE: Since this function currently does not set the _idx,
//                   please refrain from using it for indexing access.
//
//         CHAIN_ITER::CHAIN_ITER(CHAIN *sl)
//
//             Construct a doubly linked list iter.  It takes a doubly
//             linked list container and use it to initialize this
//             iterator's list pointer.  It also set cur node to NULL
//             and len/idx to -1.
//
//         CHAIN_ITER::~CHAIN_ITER(void)
//
//             Destruct a doubly linked list iter.
//
//         void CHAIN_ITER::Init(CHAIN *sl)
//
//             Uses the doubly linked list container to initialize the
//             list of this iterator.
//
//         void CHAIN_ITER::Clear(void)
//         CHAIN_NODE* CHAIN_ITER::Nth(INT n)
//
//             Gets the n-th element in the list
//             Note: Head()==Nth(0)
//
//         CHAIN_NODE* CHAIN_ITER::Last_Nth(INT n)
//
//             Gets the last n-th element in the list
//             Note: Tail()==Last_Nth(0)
//
//         CHAIN *CHAIN_ITER::List(void)
//
//             Gets the pointer to the container list
//
//         CHAIN_NODE* CHAIN_ITER::Peek_Next(void)
//
//             Gets the next element in the list
//
//         CHAIN_NODE* CHAIN_ITER::Cur(void)
//
//             Gets the "cur" element in the list
//
//         INT CHAIN_ITER::Idx(void)
class CHAIN_ITER {
private:
  CHAIN_NODE  *_cur;
  CHAIN       *_list;
  mINT16       _len;
  mINT16       _idx;          // simulate indexing, default to -1

  CHAIN_ITER& operator= (const CHAIN_ITER& sl);
  CHAIN_ITER(const CHAIN_ITER&);

protected:
  CHAIN_ITER(void)   { _list = NULL; _cur = NULL; _len = -1; _idx = -1;}
  void Set_Cur(CHAIN_NODE *cur) { _cur = cur; }

public:
  CHAIN_ITER(CHAIN *sl)
  { _list = sl; _cur=_list->Head();
    _len = -1; _idx = -1; }
  ~CHAIN_ITER(void) {}

  void         Init(CHAIN *sl) { _list = sl; _len=_idx=-1; _cur=_list->Head();}
  void         Clear(void) { _list = NULL; _cur = NULL; _len = -1; _idx = -1;}
  CHAIN_NODE  *First(void);  // get the first element, and reset the index.
  CHAIN_NODE  *Last(void);   // get the last element, and set index=len.
  CHAIN_NODE  *Next(void);   // get the next element, and incr. the pointer.
  CHAIN_NODE  *Prev(void);   // get the prev element, and decr. the pointer.
  CHAIN_NODE  *Nth(INT n);   // get the nth element
  CHAIN_NODE  *Last_Nth(INT n);// get the last nth element

  CHAIN       *List(void)            {return _list;}
  CHAIN_NODE  *Peek_Next(void)       {return _cur->Next();}
  CHAIN_NODE  *Cur(void)             {return _cur;}
  INT          Idx(void)             {return _idx;}
  INT32        Len(void);            // get the length of the list
  BOOL         Is_Empty(void)        { return _cur == NULL; }
  BOOL         Is_Empty_Reverse(void){ return _cur == NULL; }
};

#endif //OCC_COM_SLIST_H
