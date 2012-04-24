
#ifndef __RUNTIME_H
#define __RUNTIME_H

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#define __RT_HEAP_SIZE 10000

#define __CHECK_HEAP(n) /* TODO: check for heap-overflow */
#define __ALLOC1() --hp /* TODO: check for heap-overflow */
#define __ALLOC0() hp /* TODO: check for heap-overflow */
#define __ALLOCN(n) hp-=n /* TODO: check for heap-overflow */

#define __INVOKE1(o, closure)\
   ((__obj(*)(__obj))((o)->label.f))(closure)

#define __INVOKE2(o, closure, x)\
   ((__obj(*)(__obj,__obj))((o)->label.f))(closure, x)

#define __INVOKE3(o, closure, x, y)\
   ((__obj(*)(__obj,__obj,__obj))((o)->label.f))(closure, x, y)

#define __INVOKE4(o, closure, x, y, z)\
   ((__obj(*)(__obj,__obj,__obj,__obj))((o)->label.f))(closure, x, y, z)

/** ## Integers */

#define __INT_BEGIN(Cname)\
  __CHECK_HEAP(1)

#define __INT_INIT(val)\
  {__objref o = __ALLOC1();\
   o->z.header.tag = __INT;\
   o->z.value = val
   
#define __INT_END(Cname)\
   Cname = __WRAP(o);}

/** ## Labels */

#define __LABEL_BEGIN(Cname)\
  __CHECK_HEAP(1)

#define __LABEL_INIT(val)\
  {__objref o = __ALLOC1();\
   o->label.header.tag = __LABEL;\
   o->label.f = (__obj (*)(void)) val
   
#define __LABEL_END(Cname)\
   Cname = __WRAP(o);}

/** ## Closures */

#define __CLOSURE_BEGIN(Cname, n)\
   __CHECK_HEAP(n+1)

#define __CLOSURE_ADD(value)\
   {__objref o = __ALLOC1();\
    *o = *(__UNWRAP(value));}

#define __CLOSURE_END(Cname, n)\
   {__objref o = __ALLOC1();\
    o->closure.header.tag = __CLOSURE;\
    o->closure.sz = n;\
    o->closure.env = o+1;\
    Cname = __WRAP(o);}

#define __CLOSURE_REF(Cname, n) (__WRAP(&(Cname->closure.env[n])))

/** ## Records */

#define __RECORD_BEGIN(Cname, n)\
  __CHECK_HEAP(n+1)

#define __RECORD_ADD(field, value)\
  {__objref o = __ALLOC1();\
   o->tagged.header.tag = __TAGGED;\
   o->tagged.tag = field;\
   o->tagged.payload = value;}

#define __RECORD_END(Cname, n)\
  {__objref o = __ALLOC1();\
   o->record.header.tag = __RECORD;\
   o->record.sz = n;\
   o->record.fields = o+1;\
   Cname = __WRAP(o);}

#define __RECORD_BEGIN_UPDATE(Cdst, Csrc)\
  {__recordCloneFields((struct __record*)Csrc);\
   __word n = Csrc->record.sz

#define __RECORD_UPDATE(tag, value)\
   n += __recordUpdate(__ALLOC0(),n,tag,value)

#define __RECORD_END_UPDATE(Cdst)\
  {__objref o = __ALLOC1();\
   o->record.header.tag = __RECORD;\
   o->record.sz = n;\
   o->record.fields = o+1;\
   Cdst = __WRAP(o);}}

#define __RECORD_SELECT(Cname, field)\
  __recordLookup(((struct __record*)Cname), field)->tagged.payload
   
/** ## Bitvectors */

#define __BV_BEGIN(Cname, n)\
  __CHECK_HEAP(1)

#define __BV_INIT(value)\
  {__objref o = __ALLOC1();\
   o->bv.header.tag = __BV;\
   o->bv.vec = value;\

#define __BV_END(Cname, n)\
   o->bv.sz = n;\
   Cname = __WRAP(o);}

/** ## Tagged values (datatypes) */

#define __TAGGED_BEGIN(Cname)\
  __CHECK_HEAP(1)

#define __TAGGED_INIT(con, value)\
  {__objref o = __ALLOC1();\
   o->tagged.header.tag = __TAGGED;\
   o->tagged.tag = con;\
   o->tagged.payload = value;

#define __TAGGED_END(Cname)\
   Cname = __WRAP(o);}

#define __LOCAL0(Cname) __obj Cname
#define __LOCAL(Cname, body) __obj Cname = body

typedef union __header __header;
typedef union __unwrapped_obj __unwrapped_obj;
typedef union __wrapped_obj __wrapped_obj;
typedef __unwrapped_obj* __objref;
typedef __wrapped_obj* __obj;
typedef uint64_t __word;
typedef int64_t __int;

enum __tag {
  __CLOSURE,
  __BV,
  __INT,
  __TAGGED,
  __RECORD,
  __NIL,
  __BLOB,
  __LABEL
};

union __header {
  enum __tag tag;
  __obj ignored;
} __attribute__((packed,aligned(8)));

union __unwrapped_obj {
  struct __unwrapped_immediate {
    __header header;
  } object;
  struct __unwrapped_tagged {
    __header header;
    __word tag;
    __obj payload;
  } tagged;
  struct __unwrapped_closure {
    __header header;
    __word sz;
    __objref env;
  } closure;
  struct __unwrapped_record {
    __header header;
    __word sz;
    __objref fields;
  } record;
  struct __unwrapped_bv {
    __header header;
    __word sz;
    __word vec;
  } bv;
  struct __unwrapped_label {
    __header header;
    __obj (*f)(void);
  } label;
  struct __unwrapped_int {
    __header header;
    __int value;
  } z;
} __attribute__((packed,aligned(8)));

union __wrapped_obj {
  struct __tagged {
    __word tag;
    __obj payload;
  } tagged;
  struct __closure {
    __word sz;
    __objref env;
  } closure;
  struct __record {
    __word sz;
    __objref fields;
  } record;
  struct __bv {
    __word sz;
    __word vec;
  } bv;
  struct __label {
    __obj (*f)(void);
  } label;
  struct __int {
    __int value;
  } z;
} __attribute__((packed,aligned(8)));

#define __WRAP(x) ((__obj)(((__header*)x)+1))
#define __UNWRAP(x) ((__objref)(((__header*)x)-1))
#define __TAG(x) ((__UNWRAP((__obj)x))->object.header.tag)

extern void __fatal(char*,...) __attribute__((noreturn));
extern __unwrapped_obj heap[__RT_HEAP_SIZE] __attribute__((aligned(8)));
extern __objref hp;
extern __obj __UNIT;

static inline __objref __recordLookup (struct __record* record, __word field) {
  __word i, sz = record->sz;
  __objref fields = record->fields;
  for (i = 0; i < sz; i++) {
    __objref o = &fields[i];
    if (o->tagged.tag == field)
       return o;
  }
  __fatal("record-field '%lz' not found", field);
}

static inline __word __recordUpdate (__objref fields, __word n, __word field, __obj value) {
  __word i;
  for (i = 0; i < n; i++) {
    __objref o = &fields[i];
    if (o->tagged.tag == field) {
       /* Overwriting already exisiting field */
       o->tagged.payload = value;
       return 0;
    }
  }
  /* Allocating new record field */
  __objref o = __ALLOC1();
  o->tagged.header.tag = __TAGGED;
  o->tagged.tag = field;
  o->tagged.payload = value;
  return 1;
}

static inline void __recordCloneFields (struct __record* record) {
  __word sz = record->sz;
  __objref fields = __ALLOCN(sz);
  memcpy(fields, record->fields, sz*sizeof(__unwrapped_obj));
}

static inline __word __CASETAG (__obj o) {
  switch (__TAG(o)) {
    case __INT:
      return (__word)o->z.value;
    case __TAGGED:
      return o->tagged.tag;
    case __BV:
      return o->bv.vec;
    default:
      __fatal("__CASETAG() applied to non-tagged object");
  }
}

extern __obj __consume (__obj);
extern __obj __slice (__obj,__obj,__obj,__obj);
extern __obj __unconsume (__obj);

#endif /* __RUNTIME_H */