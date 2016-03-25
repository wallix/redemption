/****************************************************************************
** Meta object code from reading C++ file 'front_widget_Qt5.hpp'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.4.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../../../src/Qt5/front_widget_Qt5.hpp"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'front_widget_Qt5.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.4.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_DialogOptions_Qt_t {
    QByteArrayData data[9];
    char stringdata[108];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_DialogOptions_Qt_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_DialogOptions_Qt_t qt_meta_stringdata_DialogOptions_Qt = {
    {
QT_MOC_LITERAL(0, 0, 16), // "DialogOptions_Qt"
QT_MOC_LITERAL(1, 17, 11), // "savePressed"
QT_MOC_LITERAL(2, 29, 0), // ""
QT_MOC_LITERAL(3, 30, 12), // "saveReleased"
QT_MOC_LITERAL(4, 43, 13), // "cancelPressed"
QT_MOC_LITERAL(5, 57, 14), // "cancelReleased"
QT_MOC_LITERAL(6, 72, 6), // "addRow"
QT_MOC_LITERAL(7, 79, 13), // "deletePressed"
QT_MOC_LITERAL(8, 93, 14) // "deleteReleased"

    },
    "DialogOptions_Qt\0savePressed\0\0"
    "saveReleased\0cancelPressed\0cancelReleased\0"
    "addRow\0deletePressed\0deleteReleased"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_DialogOptions_Qt[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x0a /* Public */,
       3,    0,   50,    2, 0x0a /* Public */,
       4,    0,   51,    2, 0x0a /* Public */,
       5,    0,   52,    2, 0x0a /* Public */,
       6,    0,   53,    2, 0x0a /* Public */,
       7,    0,   54,    2, 0x0a /* Public */,
       8,    0,   55,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void DialogOptions_Qt::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        DialogOptions_Qt *_t = static_cast<DialogOptions_Qt *>(_o);
        switch (_id) {
        case 0: _t->savePressed(); break;
        case 1: _t->saveReleased(); break;
        case 2: _t->cancelPressed(); break;
        case 3: _t->cancelReleased(); break;
        case 4: _t->addRow(); break;
        case 5: _t->deletePressed(); break;
        case 6: _t->deleteReleased(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject DialogOptions_Qt::staticMetaObject = {
    { &QDialog::staticMetaObject, qt_meta_stringdata_DialogOptions_Qt.data,
      qt_meta_data_DialogOptions_Qt,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *DialogOptions_Qt::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *DialogOptions_Qt::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_DialogOptions_Qt.stringdata))
        return static_cast<void*>(const_cast< DialogOptions_Qt*>(this));
    return QDialog::qt_metacast(_clname);
}

int DialogOptions_Qt::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QDialog::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}
struct qt_meta_stringdata_Form_Qt_t {
    QByteArrayData data[6];
    char stringdata[75];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Form_Qt_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Form_Qt_t qt_meta_stringdata_Form_Qt = {
    {
QT_MOC_LITERAL(0, 0, 7), // "Form_Qt"
QT_MOC_LITERAL(1, 8, 16), // "connexionPressed"
QT_MOC_LITERAL(2, 25, 0), // ""
QT_MOC_LITERAL(3, 26, 17), // "connexionReleased"
QT_MOC_LITERAL(4, 44, 14), // "optionsPressed"
QT_MOC_LITERAL(5, 59, 15) // "optionsReleased"

    },
    "Form_Qt\0connexionPressed\0\0connexionReleased\0"
    "optionsPressed\0optionsReleased"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Form_Qt[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       4,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   34,    2, 0x08 /* Private */,
       3,    0,   35,    2, 0x08 /* Private */,
       4,    0,   36,    2, 0x08 /* Private */,
       5,    0,   37,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void Form_Qt::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Form_Qt *_t = static_cast<Form_Qt *>(_o);
        switch (_id) {
        case 0: _t->connexionPressed(); break;
        case 1: _t->connexionReleased(); break;
        case 2: _t->optionsPressed(); break;
        case 3: _t->optionsReleased(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject Form_Qt::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Form_Qt.data,
      qt_meta_data_Form_Qt,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *Form_Qt::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Form_Qt::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_Form_Qt.stringdata))
        return static_cast<void*>(const_cast< Form_Qt*>(this));
    return QWidget::qt_metacast(_clname);
}

int Form_Qt::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 4;
    }
    return _id;
}
struct qt_meta_stringdata_Screen_Qt_t {
    QByteArrayData data[9];
    char stringdata[131];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Screen_Qt_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Screen_Qt_t qt_meta_stringdata_Screen_Qt = {
    {
QT_MOC_LITERAL(0, 0, 9), // "Screen_Qt"
QT_MOC_LITERAL(1, 10, 11), // "slotRepaint"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 14), // "RefreshPressed"
QT_MOC_LITERAL(4, 38, 15), // "RefreshReleased"
QT_MOC_LITERAL(5, 54, 17), // "CtrlAltDelPressed"
QT_MOC_LITERAL(6, 72, 18), // "CtrlAltDelReleased"
QT_MOC_LITERAL(7, 91, 19), // "disconnexionPressed"
QT_MOC_LITERAL(8, 111, 19) // "disconnexionRelease"

    },
    "Screen_Qt\0slotRepaint\0\0RefreshPressed\0"
    "RefreshReleased\0CtrlAltDelPressed\0"
    "CtrlAltDelReleased\0disconnexionPressed\0"
    "disconnexionRelease"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Screen_Qt[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       7,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   49,    2, 0x08 /* Private */,
       3,    0,   50,    2, 0x08 /* Private */,
       4,    0,   51,    2, 0x08 /* Private */,
       5,    0,   52,    2, 0x08 /* Private */,
       6,    0,   53,    2, 0x08 /* Private */,
       7,    0,   54,    2, 0x08 /* Private */,
       8,    0,   55,    2, 0x08 /* Private */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void Screen_Qt::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Screen_Qt *_t = static_cast<Screen_Qt *>(_o);
        switch (_id) {
        case 0: _t->slotRepaint(); break;
        case 1: _t->RefreshPressed(); break;
        case 2: _t->RefreshReleased(); break;
        case 3: _t->CtrlAltDelPressed(); break;
        case 4: _t->CtrlAltDelReleased(); break;
        case 5: _t->disconnexionPressed(); break;
        case 6: _t->disconnexionRelease(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject Screen_Qt::staticMetaObject = {
    { &QWidget::staticMetaObject, qt_meta_stringdata_Screen_Qt.data,
      qt_meta_data_Screen_Qt,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *Screen_Qt::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Screen_Qt::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_Screen_Qt.stringdata))
        return static_cast<void*>(const_cast< Screen_Qt*>(this));
    return QWidget::qt_metacast(_clname);
}

int Screen_Qt::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 7;
    }
    return _id;
}
struct qt_meta_stringdata_Connector_Qt_t {
    QByteArrayData data[4];
    char stringdata[38];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_Connector_Qt_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_Connector_Qt_t qt_meta_stringdata_Connector_Qt = {
    {
QT_MOC_LITERAL(0, 0, 12), // "Connector_Qt"
QT_MOC_LITERAL(1, 13, 9), // "call_Draw"
QT_MOC_LITERAL(2, 23, 0), // ""
QT_MOC_LITERAL(3, 24, 13) // "mem_clipboard"

    },
    "Connector_Qt\0call_Draw\0\0mem_clipboard"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_Connector_Qt[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
       2,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   24,    2, 0x0a /* Public */,
       3,    0,   25,    2, 0x0a /* Public */,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,

       0        // eod
};

void Connector_Qt::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Connector_Qt *_t = static_cast<Connector_Qt *>(_o);
        switch (_id) {
        case 0: _t->call_Draw(); break;
        case 1: _t->mem_clipboard(); break;
        default: ;
        }
    }
    Q_UNUSED(_a);
}

const QMetaObject Connector_Qt::staticMetaObject = {
    { &QObject::staticMetaObject, qt_meta_stringdata_Connector_Qt.data,
      qt_meta_data_Connector_Qt,  qt_static_metacall, Q_NULLPTR, Q_NULLPTR}
};


const QMetaObject *Connector_Qt::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Connector_Qt::qt_metacast(const char *_clname)
{
    if (!_clname) return Q_NULLPTR;
    if (!strcmp(_clname, qt_meta_stringdata_Connector_Qt.stringdata))
        return static_cast<void*>(const_cast< Connector_Qt*>(this));
    return QObject::qt_metacast(_clname);
}

int Connector_Qt::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 2)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 2)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 2;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
