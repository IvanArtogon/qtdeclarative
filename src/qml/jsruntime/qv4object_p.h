/****************************************************************************
**
** Copyright (C) 2015 The Qt Company Ltd.
** Contact: http://www.qt.io/licensing/
**
** This file is part of the QtQml module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL21$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see http://www.qt.io/terms-conditions. For further
** information use the contact form at http://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 or version 3 as published by the Free
** Software Foundation and appearing in the file LICENSE.LGPLv21 and
** LICENSE.LGPLv3 included in the packaging of this file. Please review the
** following information to ensure the GNU Lesser General Public License
** requirements will be met: https://www.gnu.org/licenses/lgpl.html and
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** As a special exception, The Qt Company gives you certain additional
** rights. These rights are described in The Qt Company LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** $QT_END_LICENSE$
**
****************************************************************************/
#ifndef QV4_OBJECT_H
#define QV4_OBJECT_H

#include "qv4managed_p.h"
#include "qv4memberdata_p.h"
#include "qv4arraydata_p.h"
#include "qv4engine_p.h"
#include "qv4scopedvalue_p.h"
#include "qv4value_p.h"

QT_BEGIN_NAMESPACE

namespace QV4 {

namespace Heap {

struct Object : Base {
    inline Object() {}
    inline Object(ExecutionEngine *engine);
    Object(InternalClass *internal, QV4::Object *prototype);

    const Value *propertyData(uint index) const { if (index < inlineMemberSize) return reinterpret_cast<const Value *>(this) + inlineMemberOffset + index; return memberData->data + index - inlineMemberSize; }
    Value *propertyData(uint index) { if (index < inlineMemberSize) return reinterpret_cast<Value *>(this) + inlineMemberOffset + index; return memberData->data + index - inlineMemberSize; }

    uint inlineMemberOffset;
    uint inlineMemberSize;
    InternalClass *internalClass;
    Pointer<Object> prototype;
    Pointer<MemberData> memberData;
    Pointer<ArrayData> arrayData;
};

}

#define V4_OBJECT(superClass) \
    public: \
        Q_MANAGED_CHECK \
        typedef superClass SuperClass; \
        static const QV4::ObjectVTable static_vtbl; \
        static inline const QV4::VTable *staticVTable() { return &static_vtbl.vTable; } \
        V4_MANAGED_SIZE_TEST \
        Data *d() const { return static_cast<Data *>(m()); }

#define V4_OBJECT2(DataClass, superClass) \
    public: \
        Q_MANAGED_CHECK \
        typedef QV4::Heap::DataClass Data; \
        typedef superClass SuperClass; \
        static const QV4::ObjectVTable static_vtbl; \
        static inline const QV4::VTable *staticVTable() { return &static_vtbl.vTable; } \
        V4_MANAGED_SIZE_TEST \
        QV4::Heap::DataClass *d() const { return static_cast<QV4::Heap::DataClass *>(m()); }

struct ObjectVTable
{
    VTable vTable;
    ReturnedValue (*call)(const Managed *, CallData *data);
    ReturnedValue (*construct)(const Managed *, CallData *data);
    ReturnedValue (*get)(const Managed *, String *name, bool *hasProperty);
    ReturnedValue (*getIndexed)(const Managed *, uint index, bool *hasProperty);
    void (*put)(Managed *, String *name, const Value &value);
    void (*putIndexed)(Managed *, uint index, const Value &value);
    PropertyAttributes (*query)(const Managed *, String *name);
    PropertyAttributes (*queryIndexed)(const Managed *, uint index);
    bool (*deleteProperty)(Managed *m, String *name);
    bool (*deleteIndexedProperty)(Managed *m, uint index);
    ReturnedValue (*getLookup)(const Managed *m, Lookup *l);
    void (*setLookup)(Managed *m, Lookup *l, const Value &v);
    uint (*getLength)(const Managed *m);
    void (*advanceIterator)(Managed *m, ObjectIterator *it, Value *name, uint *index, Property *p, PropertyAttributes *attributes);
};

#define DEFINE_OBJECT_VTABLE(classname) \
const QV4::ObjectVTable classname::static_vtbl =    \
{     \
    DEFINE_MANAGED_VTABLE_INT(classname, &classname::SuperClass::static_vtbl == &Object::static_vtbl ? 0 : &classname::SuperClass::static_vtbl.vTable), \
    call,                                       \
    construct,                                  \
    get,                                        \
    getIndexed,                                 \
    put,                                        \
    putIndexed,                                 \
    query,                                      \
    queryIndexed,                               \
    deleteProperty,                             \
    deleteIndexedProperty,                      \
    getLookup,                                  \
    setLookup,                                  \
    getLength,                                  \
    advanceIterator                            \
}



struct Q_QML_EXPORT Object: Managed {
    V4_OBJECT2(Object, Object)
    Q_MANAGED_TYPE(Object)

    enum {
        IsObject = true,
        GetterOffset = 0,
        SetterOffset = 1
    };

    InternalClass *internalClass() const { return d()->internalClass; }
    void setInternalClass(InternalClass *ic);

    const Value *propertyData(uint index) const { return d()->propertyData(index); }
    Value *propertyData(uint index) { return d()->propertyData(index); }

    Heap::ArrayData *arrayData() const { return d()->arrayData; }
    void setArrayData(ArrayData *a) { d()->arrayData = a->d(); }

    void getProperty(uint index, Property *p, PropertyAttributes *attrs) const;
    void setProperty(uint index, const Property *p);

    const ObjectVTable *vtable() const { return reinterpret_cast<const ObjectVTable *>(d()->vtable()); }
    Heap::Object *prototype() const { return d()->prototype; }
    bool setPrototype(Object *proto);

    void getOwnProperty(String *name, PropertyAttributes *attrs, Property *p = 0);
    void getOwnProperty(uint index, PropertyAttributes *attrs, Property *p = 0);

    Value *getValueOrSetter(String *name, PropertyAttributes *attrs);
    Value *getValueOrSetter(uint index, PropertyAttributes *attrs);

    bool hasProperty(String *name) const;
    bool hasProperty(uint index) const;

    bool hasOwnProperty(String *name) const;
    bool hasOwnProperty(uint index) const;

    bool __defineOwnProperty__(ExecutionEngine *engine, uint index, String *member, const Property *p, PropertyAttributes attrs);
    bool __defineOwnProperty__(ExecutionEngine *engine, String *name, const Property *p, PropertyAttributes attrs);
    bool __defineOwnProperty__(ExecutionEngine *engine, uint index, const Property *p, PropertyAttributes attrs);
    bool __defineOwnProperty__(ExecutionEngine *engine, const QString &name, const Property *p, PropertyAttributes attrs);
    bool defineOwnProperty2(ExecutionEngine *engine, uint index, const Property *p, PropertyAttributes attrs);

    //
    // helpers
    //
    void put(ExecutionEngine *engine, const QString &name, const Value &value);

    static ReturnedValue getValue(const Value &thisObject, const Value &v, PropertyAttributes attrs);
    ReturnedValue getValue(const Value &v, PropertyAttributes attrs) const {
        Scope scope(this->engine());
        ScopedValue t(scope, const_cast<Object *>(this));
        return getValue(t, v, attrs);
    }

    void putValue(uint memberIndex, const Value &value);

    /* The spec default: Writable: true, Enumerable: false, Configurable: true */
    void defineDefaultProperty(String *name, const Value &value) {
        insertMember(name, value, Attr_Data|Attr_NotEnumerable);
    }
    void defineDefaultProperty(const QString &name, const Value &value);
    void defineDefaultProperty(const QString &name, ReturnedValue (*code)(CallContext *), int argumentCount = 0);
    void defineDefaultProperty(String *name, ReturnedValue (*code)(CallContext *), int argumentCount = 0);
    void defineAccessorProperty(const QString &name, ReturnedValue (*getter)(CallContext *), ReturnedValue (*setter)(CallContext *));
    void defineAccessorProperty(String *name, ReturnedValue (*getter)(CallContext *), ReturnedValue (*setter)(CallContext *));
    /* Fixed: Writable: false, Enumerable: false, Configurable: false */
    void defineReadonlyProperty(const QString &name, const Value &value);
    void defineReadonlyProperty(String *name, const Value &value);

    void insertMember(String *s, const Value &v, PropertyAttributes attributes = Attr_Data) {
        Scope scope(engine());
        ScopedProperty p(scope);
        p->value = v;
        insertMember(s, p, attributes);
    }
    void insertMember(String *s, const Property *p, PropertyAttributes attributes);

    inline ExecutionEngine *engine() const { return internalClass()->engine; }

    bool isExtensible() const { return d()->internalClass->extensible; }

    // Array handling

public:
    void copyArrayData(Object *other);

    bool setArrayLength(uint newLen);
    void setArrayLengthUnchecked(uint l);

    void arraySet(uint index, const Property *p, PropertyAttributes attributes = Attr_Data);
    void arraySet(uint index, const Value &value);

    bool arrayPut(uint index, const Value &value) {
        return arrayData()->vtable()->put(this, index, value);
    }
    bool arrayPut(uint index, const Value *values, uint n) {
        return arrayData()->vtable()->putArray(this, index, values, n);
    }
    void setArrayAttributes(uint i, PropertyAttributes a) {
        Q_ASSERT(arrayData());
        if (d()->arrayData->attrs || a != Attr_Data) {
            ArrayData::ensureAttributes(this);
            a.resolve();
            arrayData()->vtable()->setAttribute(this, i, a);
        }
    }

    void push_back(const Value &v);

    ArrayData::Type arrayType() const {
        return arrayData() ? d()->arrayData->type : Heap::ArrayData::Simple;
    }
    // ### remove me
    void setArrayType(ArrayData::Type t) {
        Q_ASSERT(t != Heap::ArrayData::Simple && t != Heap::ArrayData::Sparse);
        arrayCreate();
        d()->arrayData->type = t;
    }

    inline void arrayReserve(uint n) {
        ArrayData::realloc(this, Heap::ArrayData::Simple, n, false);
    }

    void arrayCreate() {
        if (!arrayData())
            ArrayData::realloc(this, Heap::ArrayData::Simple, 0, false);
#ifdef CHECK_SPARSE_ARRAYS
        initSparseArray();
#endif
    }

    void initSparseArray();
    SparseArrayNode *sparseBegin() { return arrayType() == Heap::ArrayData::Sparse ? d()->arrayData->sparse->begin() : 0; }
    SparseArrayNode *sparseEnd() { return arrayType() == Heap::ArrayData::Sparse ? d()->arrayData->sparse->end() : 0; }

    inline bool protoHasArray() {
        Scope scope(engine());
        ScopedObject p(scope, this);

        while ((p = p->prototype()))
            if (p->arrayData())
                return true;

        return false;
    }

    inline ReturnedValue get(String *name, bool *hasProperty = 0) const
    { return vtable()->get(this, name, hasProperty); }
    inline ReturnedValue getIndexed(uint idx, bool *hasProperty = 0) const
    { return vtable()->getIndexed(this, idx, hasProperty); }
    inline void put(String *name, const Value &v)
    { vtable()->put(this, name, v); }
    inline void putIndexed(uint idx, const Value &v)
    { vtable()->putIndexed(this, idx, v); }
    PropertyAttributes query(String *name) const
    { return vtable()->query(this, name); }
    PropertyAttributes queryIndexed(uint index) const
    { return vtable()->queryIndexed(this, index); }
    bool deleteProperty(String *name)
    { return vtable()->deleteProperty(this, name); }
    bool deleteIndexedProperty(uint index)
    { return vtable()->deleteIndexedProperty(this, index); }
    ReturnedValue getLookup(Lookup *l) const
    { return vtable()->getLookup(this, l); }
    void setLookup(Lookup *l, const Value &v)
    { vtable()->setLookup(this, l, v); }
    void advanceIterator(ObjectIterator *it, Value *name, uint *index, Property *p, PropertyAttributes *attributes)
    { vtable()->advanceIterator(this, it, name, index, p, attributes); }
    uint getLength() const { return vtable()->getLength(this); }

    inline ReturnedValue construct(CallData *d) const
    { return vtable()->construct(this, d); }
    inline ReturnedValue call(CallData *d) const
    { return vtable()->call(this, d); }
protected:
    static void markObjects(Heap::Base *that, ExecutionEngine *e);
    static ReturnedValue construct(const Managed *m, CallData *);
    static ReturnedValue call(const Managed *m, CallData *);
    static ReturnedValue get(const Managed *m, String *name, bool *hasProperty);
    static ReturnedValue getIndexed(const Managed *m, uint index, bool *hasProperty);
    static void put(Managed *m, String *name, const Value &value);
    static void putIndexed(Managed *m, uint index, const Value &value);
    static PropertyAttributes query(const Managed *m, String *name);
    static PropertyAttributes queryIndexed(const Managed *m, uint index);
    static bool deleteProperty(Managed *m, String *name);
    static bool deleteIndexedProperty(Managed *m, uint index);
    static ReturnedValue getLookup(const Managed *m, Lookup *l);
    static void setLookup(Managed *m, Lookup *l, const Value &v);
    static void advanceIterator(Managed *m, ObjectIterator *it, Value *name, uint *index, Property *p, PropertyAttributes *attributes);
    static uint getLength(const Managed *m);

    void ensureMemberData();

private:
    ReturnedValue internalGet(String *name, bool *hasProperty) const;
    ReturnedValue internalGetIndexed(uint index, bool *hasProperty) const;
    void internalPut(String *name, const Value &value);
    void internalPutIndexed(uint index, const Value &value);
    bool internalDeleteProperty(String *name);
    bool internalDeleteIndexedProperty(uint index);

    friend struct ObjectIterator;
    friend struct ObjectPrototype;
};

namespace Heap {

inline Object::Object(ExecutionEngine *engine)
    : internalClass(engine->emptyClass),
      prototype(static_cast<Object *>(engine->objectPrototype()->m()))
{
}

struct BooleanObject : Object {
    BooleanObject() {}
    BooleanObject(bool b)
        : b(b)
    {}
    bool b;
};

struct NumberObject : Object {
    NumberObject() {}
    NumberObject(double val)
        : value(val)
    {}
    double value;
};

struct ArrayObject : Object {
    enum {
        LengthPropertyIndex = 0
    };

    ArrayObject()
    { init(); }
    ArrayObject(const QStringList &list);
    void init()
    { *propertyData(LengthPropertyIndex) = Primitive::fromInt32(0); }
};

}

struct BooleanObject: Object {
    V4_OBJECT2(BooleanObject, Object)
    Q_MANAGED_TYPE(BooleanObject)

    bool value() const { return d()->b; }

};

struct NumberObject: Object {
    V4_OBJECT2(NumberObject, Object)
    Q_MANAGED_TYPE(NumberObject)

    double value() const { return d()->value; }
};

struct ArrayObject: Object {
    V4_OBJECT2(ArrayObject, Object)
    Q_MANAGED_TYPE(ArrayObject)

    void init(ExecutionEngine *engine);

    static ReturnedValue getLookup(const Managed *m, Lookup *l);
    using Object::getLength;
    static uint getLength(const Managed *m);

    QStringList toQStringList() const;
};

inline void Object::setArrayLengthUnchecked(uint l)
{
    if (isArrayObject())
        *propertyData(Heap::ArrayObject::LengthPropertyIndex) = Primitive::fromUInt32(l);
}

inline void Object::push_back(const Value &v)
{
    arrayCreate();

    uint idx = getLength();
    arrayReserve(idx + 1);
    arrayPut(idx, v);
    setArrayLengthUnchecked(idx + 1);
}

inline void Object::arraySet(uint index, const Property *p, PropertyAttributes attributes)
{
    // ### Clean up
    arrayCreate();
    if (attributes.isAccessor() || (index > 0x1000 && index > 2*d()->arrayData->alloc)) {
        initSparseArray();
    } else {
        arrayData()->vtable()->reallocate(this, index + 1, false);
    }
    setArrayAttributes(index, attributes);
    ArrayData::insert(this, index, &p->value, attributes.isAccessor());
    if (isArrayObject() && index >= getLength())
        setArrayLengthUnchecked(index + 1);
}


inline void Object::arraySet(uint index, const Value &value)
{
    arrayCreate();
    if (index > 0x1000 && index > 2*d()->arrayData->alloc) {
        initSparseArray();
    }
    ArrayData::insert(this, index, &value);
    if (isArrayObject() && index >= getLength())
        setArrayLengthUnchecked(index + 1);
}


template<>
inline const ArrayObject *Value::as() const {
    return isManaged() && m() && m()->vtable()->type == Managed::Type_ArrayObject ? static_cast<const ArrayObject *>(this) : 0;
}

#ifndef V4_BOOTSTRAP
template<>
inline ReturnedValue value_convert<Object>(ExecutionEngine *e, const Value &v)
{
    return v.toObject(e)->asReturnedValue();
}
#endif

}

QT_END_NAMESPACE

#endif // QMLJS_OBJECTS_H
