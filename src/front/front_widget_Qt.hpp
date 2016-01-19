/*
 * 
 */

#ifndef FORM_QT_HPP
#define FORM_QT_HPP


#include "socket_transport.hpp"
#include "rdp/rdp.hpp"
#include "../src/front/front_Qt.hpp"

class Form_Qt : public QWidget
{
    
Q_OBJECT    
    
public:
    Front_Qt_API     * _front;
    QFormLayout          _formLayout;
    QLineEdit            _userNameField;
    QLineEdit            _IPField; 
    QLineEdit            _PWDField;
    QLineEdit            _portField;
    QPushButton          _buttonConnexion;
    QLabel               _userNameLabel;           
    QLabel               _IPLabel;  
    QLabel               _PWDLabel;  
    QLabel               _portLabel;
    
    
    Form_Qt(Front_Qt_API * front)
        : QWidget()
        , _front(front)
        , _formLayout(this)
        , _userNameField("", this)
        , _IPField("", this)
        , _PWDField("", this)
        , _portField("", this)
        , _buttonConnexion("Connexion", this)
        , _userNameLabel(QString("User name : "), this)         
        , _IPLabel(QString      ("IP serveur :"), this)   
        , _PWDLabel(QString  ("Password :  "), this)   
        , _portLabel(QString    ("Port :      "), this) 
    {
        this->setAttribute( Qt::WA_DeleteOnClose );
        this->_PWDField.setEchoMode(QLineEdit::Password);
        this->_PWDField.setInputMethodHints(Qt::ImhHiddenText | Qt::ImhNoPredictiveText | Qt::ImhNoAutoUppercase);
        this->setFixedSize(400, 300);
        this->_formLayout.addRow(&(this->_IPLabel)      , &(this->_IPField));
        this->_formLayout.addRow(&(this->_userNameLabel), &(this->_userNameField));
        this->_formLayout.addRow(&(this->_PWDLabel)     , &(this->_PWDField));
        this->_formLayout.addRow(&(this->_portLabel)    , &(this->_portField));
        this->setLayout(&(this->_formLayout));
        
        QRect rectConnexion(QPoint(280, 256), QSize(110, 24)); 
        this->_front->initButton(this->_buttonConnexion, "Connexion", rectConnexion);
        this->QObject::connect(&(this->_buttonConnexion)   , SIGNAL (pressed()),  this, SLOT (connexionPressed()));
        this->QObject::connect(&(this->_buttonConnexion)   , SIGNAL (released()), this, SLOT (connexionRelease()));
    }
    
    ~Form_Qt() {
        this->_front->closeFromForm();
    }
    
    
    void set_IPField(std::string str) {
        this->_IPField.clear();
        this->_IPField.insert(QString(str.c_str()));
    }
    
    
    void set_userNameField(std::string str) {
        this->_userNameField.clear();
        this->_userNameField.insert(QString(str.c_str()));
    }
    
    
    void set_PWDField(std::string str) {
        this->_PWDField.clear();
        this->_PWDField.insert(QString(str.c_str()));
    }
    
    
    void set_portField(int str) {
        this->_portField.clear();
        this->_portField.insert(QString(std::to_string(str).c_str()));
    }
    
    
    std::string get_IPField() {
        return this->_IPField.text().toStdString();
    }
    
    
    std::string get_userNameField() {
        return this->_userNameField.text().toStdString();
    }
    
    
    std::string get_PWDField() {
        return this->_PWDField.text().toStdString();
    }
    
    
    int get_portField() {;
        return this->_portField.text().toInt();
    }
    
    
private Q_SLOTS:
    void connexionPressed() {
        this->_front->connexionPressed();
    }
    
    void connexionRelease() {
        this->_front->connexionRelease();
    }
};




class Screen_Qt : public QWidget
{
    
Q_OBJECT
    
public:
    Front_Qt_API       * _front;
    QLabel               _label;
    QPicture             _picture;
    QPen                 _pen;
    QPainter             _painter;
    QPushButton          _buttonCtrlAltDel;
    QPushButton          _buttonRefresh;
    QPushButton          _buttonDisconnexion;
    
    Screen_Qt (Front_Qt_API * front)
    : QWidget()
    , _front(front)
    , _label(this)
    , _picture() 
    , _pen() 
    , _painter()
    , _buttonCtrlAltDel("CTRL + ALT + DELETE", this)
    , _buttonRefresh("Refresh", this)
    , _buttonDisconnexion("Disconnexion", this)
    {
        this->setSize();
        
        this->_painter.setRenderHint(QPainter::Antialiasing);
        this->_pen.setWidth(1);
        this->_painter.setPen(this->_pen);
        this->_label.setMouseTracking(true);
        this->_label.installEventFilter(this);
        this->setAttribute(Qt::WA_NoSystemBackground, true);
        this->setAttribute( Qt::WA_DeleteOnClose );
    
        QRect rectCtrlAltDel(QPoint(0, this->_front->info.height+1),QSize(this->_front->info.width/3, 20));
        this->_front->initButton(this->_buttonCtrlAltDel, "CTRL + ALT + DELETE", rectCtrlAltDel);
        this->QObject::connect(&(this->_buttonCtrlAltDel)  , SIGNAL (pressed()),  this, SLOT (CtrlAltDelPressed()));
        this->QObject::connect(&(this->_buttonCtrlAltDel)  , SIGNAL (released()), this, SLOT (CtrlAltDelReleased()));

        QRect rectRefresh(QPoint(this->_front->info.width/3, this->_front->info.height+1),QSize(this->_front->info.width/3, 20));
        this->_front->initButton(this->_buttonRefresh, "Refresh", rectRefresh);
        this->QObject::connect(&(this->_buttonRefresh)     , SIGNAL (pressed()),  this, SLOT (RefreshPressed()));
        this->QObject::connect(&(this->_buttonRefresh)     , SIGNAL (released()), this, SLOT (RefreshReleased()));
        
        QRect rectDisconnexion(QPoint(((this->_front->info.width/3)*2), this->_front->info.height+1),QSize(this->_front->info.width-((this->_front->info.width/3)*2), 20));
        this->_front->initButton(this->_buttonDisconnexion, "Disconnexion", rectDisconnexion);
        this->QObject::connect(&(this->_buttonDisconnexion), SIGNAL (pressed()),  this, SLOT (disconnexionPressed()));
        this->QObject::connect(&(this->_buttonDisconnexion), SIGNAL (released()), this, SLOT (disconnexionRelease()));
        
        this->reInitView();
        this->_painter.fillRect(0, 0, this->_front->info.width, this->_front->info.height, Qt::white);
        this->flush();
        
        this->setFocusPolicy(Qt::StrongFocus);
        this->reInitView();
    }
    
    ~Screen_Qt() {
        this->_front->closeFromScreen();
    }
    
    void setSize() {
        this->setFixedSize(this->_front->info.width, this->_front->info.height+20);   
    }
    
    void reInitView() {
        this->_painter.begin(&(this->_picture));
        this->_painter.fillRect(0, 0, this->_front->info.width, this->_front->info.height, QColor(0, 0, 0, 0));
    }
    
    void flush() {
        this->_painter.end();
        this->_label.setPicture(this->_picture);
        this->show();
    }
    
    QPen * getPen() {
        return &_pen;
    }
    
    QPainter * order() {
        return &_painter;
    }
    
private:
    void mousePressEvent(QMouseEvent *e) {
        this->_front->mousePressEvent(e);
    }
    
    void mouseReleaseEvent(QMouseEvent *e) {
        this->_front->mouseReleaseEvent(e);
    }
    
    void keyPressEvent(QKeyEvent *e) { 
        this->_front->keyPressEvent(e);
    }
    
    void keyReleaseEvent(QKeyEvent *e) {
        this->_front->keyReleaseEvent(e);
    }
    
    void wheelEvent(QWheelEvent *e) {
        this->_front->wheelEvent(e);
    }
    
    bool eventFilter(QObject *obj, QEvent *e) {
        this->_front->eventFilter(obj, e);
    }
    
    
private Q_SLOTS:
    void RefreshPressed() {
        this->_front->RefreshPressed();
    }
    
    void RefreshReleased() {
        this->_front->RefreshReleased();
    }
    
    void CtrlAltDelPressed() {
        this->_front->CtrlAltDelPressed();    
    }
    
    void CtrlAltDelReleased() {
        this->_front->CtrlAltDelReleased();
    }
    
    void disconnexionPressed() {
        this->_front->disconnexionPressed();
    }
    
    void disconnexionRelease(){
        this->_front->disconnexionRelease();
    }
};



class Connector_Qt : public QObject
{
    
    Q_OBJECT
    
public:
    Front_Qt_API    * _front;
    QSocketNotifier * _sckRead;
    mod_api         * _callback;
    SocketTransport * _sck;
    int               _client_sck;
    
    
    Connector_Qt(Front_Qt_API * front) 
    : QObject()
    , _front(front)
    , _sckRead(nullptr)
    , _callback(nullptr)
    , _sck(nullptr)
    , _client_sck(0)
    {}
    
    ~Connector_Qt() {
        if (this->_callback != nullptr) {
            this->_callback->send_disconnect_ultimatum();
        }
        this->dropConnect();
    }
    
    void dropConnect() {
     if (this->_sckRead != nullptr) {
            delete (this->_sckRead);
            this->_sckRead = nullptr;
        }
        if (this->_callback != nullptr) {
            delete (this->_callback);
            this->_callback = nullptr;
        }
        if (this->_sck != nullptr) {
            delete (this->_sck);
            this->_callback = nullptr;
        }   
    }
    
    bool connect() {
        const char * name(this->_front->_userName.c_str());      
        const char * pwd(this->_front->_pwd.c_str()); 
        const char * targetIP(this->_front->_targetIP.c_str());         
        
        std::cout << name << " " << pwd << " " << targetIP << " " << this->_front->_port << std::endl;
        bool changeView(false);
        
        this->_client_sck = ip_connect(targetIP, this->_front->_port, this->_front->_nbTry, this->_front->_retryDelay, this->_front->verbose);
        
        if (this->_client_sck > 0) {
            try {
                std::string error_message;
                this->_sck = new SocketTransport( name
                                                , this->_client_sck
                                                , targetIP
                                                , this->_front->_port
                                                , this->_front->verbose
                                                , &error_message
                                                );
                changeView = true;
                
            } catch (const std::exception & e) {
                std::cout << "Can not connect to [" << targetIP <<  "]." << std::endl;
                this->_front->disconnect();
            }
        } else {
            std::cout << "Can not connect to [" << targetIP <<  "]." << std::endl;
            this->_front->disconnect();
        }
        
        return changeView;
        
    }
    
    void listen() {
        const char * name(this->_front->_userName.c_str());      
        const char * pwd(this->_front->_pwd.c_str()); 
        const char * targetIP(this->_front->_targetIP.c_str());         
        const char * localIP(this->_front->_localIP.c_str());
        
        Inifile ini;
        ModRDPParams mod_rdp_params( name
                                    , pwd
                                    , targetIP
                                    , localIP
                                    , 2
                                    , 0
                                    );  
        mod_rdp_params.device_id                       = "device_id";
        mod_rdp_params.enable_tls                      = false;
        mod_rdp_params.enable_nla                      = false;
        //mod_rdp_params.enable_krb                      = false;
        //mod_rdp_params.enable_clipboard                = true;
        mod_rdp_params.enable_fastpath                 = false; 
        mod_rdp_params.enable_mem3blt                  = false;
        mod_rdp_params.enable_bitmap_update            = true;
        mod_rdp_params.enable_new_pointer              = false;
        //mod_rdp_params.rdp_compression                 = 0;
        //mod_rdp_params.error_message                   = nullptr;
        //mod_rdp_params.disconnect_on_logon_user_change = false;
        //mod_rdp_params.open_session_timeout            = 0;
        //mod_rdp_params.certificate_change_action       = 0;
        //mod_rdp_params.extra_orders                    = "";
        mod_rdp_params.server_redirection_support        = true;
    
        LCGRandom gen(0); // To always get the same client random, in tests
        this->_callback = new mod_rdp(*(this->_sck), *(this->_front), this->_front->info, ini.get_ref<cfg::mod_rdp::redir_info>(), gen, mod_rdp_params);
        this->_front->_callback = this->_callback;
        this->_sckRead = new QSocketNotifier(this->_client_sck, QSocketNotifier::Read, this);
        this->QObject::connect(this->_sckRead, SIGNAL(activated(int)), this, SLOT(call_Draw()));
    }
    
public Q_SLOTS:
    void call_Draw() {
        this->_front->call_Draw();
    }
    
};

#endif