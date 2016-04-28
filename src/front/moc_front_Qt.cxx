/****************************************************************************
** Meta object code from reading C++ file 'front_Qt.hpp'
**
** Created by: The Qt Meta Object Compiler version 63 (Qt 4.8.6)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "front_Qt.hpp"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'front_Qt.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.6. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_Front_Qt[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       1,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
      10,    9,    9,    9, 0x0a,

       0        // eod
};

static const char qt_meta_stringdata_Front_Qt[] = {
    "Front_Qt\0\0readSck_And_ShowView()\0"
};

void Front_Qt::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        Front_Qt *_t = static_cast<Front_Qt *>(_o);
        switch (_id) {
        case 0: _t->readSck_And_ShowView(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObjectExtraData Front_Qt::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject Front_Qt::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Front_Qt,
      qt_meta_data_Front_Qt, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &Front_Qt::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *Front_Qt::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *Front_Qt::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_Front_Qt))
        return static_cast<void*>(const_cast< Front_Qt*>(this));
    if (!strcmp(_clname, "FrontAPI"))
        return static_cast< FrontAPI*>(const_cast< Front_Qt*>(this));
    return QWidget::qt_metacast(_clname);
}

int Front_Qt::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 1)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 1;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
