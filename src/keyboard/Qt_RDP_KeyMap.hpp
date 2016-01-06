/*
 *
 */

#ifndef QT_RDP_KEYMAPHPP
#define QT_RDP_KEYMAPHPP
   

#include <iostream>

#include "reversed_keymaps/keylayouts_r.hpp"

#include <QtGui/QKeyEvent>



static const Keylayout_r * keylayoutsList[] = { &keylayout_x80000405, &keylayout_x80000406, &keylayout_x80000407
                                              , &keylayout_x80000408, &keylayout_x80000409, &keylayout_x8000040a
                                              , &keylayout_x8000040b, &keylayout_x8000040c, &keylayout_x8000040f
                                              , &keylayout_x80000410, &keylayout_x80000413, &keylayout_x80000414
                                              , &keylayout_x80000415, &keylayout_x80000416, &keylayout_x80000418
                                              , &keylayout_x80000419, &keylayout_x8000041a, &keylayout_x8000041b
                                              , &keylayout_x8000041d, &keylayout_x8000041f, &keylayout_x80000422
                                              , &keylayout_x80000424, &keylayout_x80000425, &keylayout_x80000426
                                              , &keylayout_x80000427, &keylayout_x8000042f, &keylayout_x80000438
                                              , &keylayout_x8000043a, &keylayout_x8000043b, &keylayout_x8000043f
                                              , &keylayout_x80000440, &keylayout_x80000444, &keylayout_x80000450
                                              , &keylayout_x80000452, &keylayout_x8000046e, &keylayout_x80000481
                                              , &keylayout_x80000807, &keylayout_x80000809, &keylayout_x8000080a
                                              , &keylayout_x8000080c, &keylayout_x80000813, &keylayout_x80000816
                                              , &keylayout_x8000081a, &keylayout_x8000083b, &keylayout_x80000843
                                              , &keylayout_x8000085d, &keylayout_x80000c0c, &keylayout_x80000c1a
                                              , &keylayout_x80001009, &keylayout_x8000100c, &keylayout_x8000201a
                                              , &keylayout_x80010402, &keylayout_x80010405, &keylayout_x80001809
                                              , &keylayout_x80010407, &keylayout_x80010408, &keylayout_x8001040a
                                              , &keylayout_x8001040e, &keylayout_x80010409, &keylayout_x80010410
                                              , &keylayout_x80010415, &keylayout_x80010416, &keylayout_x80010419
                                              , &keylayout_x8001041b, &keylayout_x8001041f, &keylayout_x80010426
                                              , &keylayout_x80010427, &keylayout_x8001043a, &keylayout_x8001043b
                                              , &keylayout_x8001080c, &keylayout_x8001083b, &keylayout_x80011009
                                              , &keylayout_x80011809, &keylayout_x80020405, &keylayout_x80020408
                                              , &keylayout_x80020409, &keylayout_x8002083b, &keylayout_x80030402
                                              , &keylayout_x80030408, &keylayout_x80030409, &keylayout_x80040408
                                              , &keylayout_x80040409, &keylayout_x80050408, &keylayout_x80060408
                                              };                                    
                                        
                                        

class Qt_RDP_KeyMap
{
    
private:
    int                                 _verbose;
    uint8_t                             _keyboardMods;
    const Keylayout_r *                 _keylayout_WORK;
    const Keylayout_r::KeyLayoutMap_t * _layoutMods[9];
    const Keylayout_r::KeyLayoutMap_t * _layout;
    
    Keylayout_r::KeyLayoutMap_t         _customNoExtended;
    Keylayout_r::KeyLayoutMap_t         _customExtended;
    Keylayout_r::KeyLayoutMap_t         _customNoExtendedKeylayoutApplied;
    Keylayout_r::KeyLayoutMap_t         _customExtendedKeylayoutApplied;
    
    bool                                _deadKeys;
    
    
    
    enum {
          ALT_MOD      = 0x10
        , CTRL_MOD     = 0x08
        , CAPSLOCK_MOD = 0x04
        , ALTGR_MOD    = 0x02
        , SHIFT_MOD    = 0x01
    };
    
    enum {
          MAX_CHAR_QT_KEY_VALUE     = 256 
        , MIN_NON_CHAR_QT_KEY_VALUE = 0x00FFFFFF       
        , MIN_DEAD_QT_KEY_VALUE     = 0x01001249
        , MAX_DEAD_QT_KEY_VALUE     = 0x01001263
        , MASK_ALT                  = 0x0F
    };
    
    enum {
           KBDFLAGS_EXTENDED = 0x0100
         , KBDFLAGS_DOWN     = 0x4000
         , KBDFLAGS_RELEASE  = 0x8000
    };
    
    
    
    void applyCharTable() {
        try {
            if (this->_deadKeys) {
                const Keylayout_r::KeyLayoutMap_t & layout = this->_keylayout_WORK->deadkeys;
                this->keyCode = layout.at(keyCode);
            } else {
                const Keylayout_r::KeyLayoutMap_t & layout = *(this->_layout);
                this->keyCode = layout.at(keyCode);
            }
        } catch (const std::exception & e) {
            std::cerr << "Unknown key(" << this->keyCode << ") to current layout 0x" << std::hex << this->_keylayout_WORK->LCID << " " << this->_keylayout_WORK->locale_name << "." << std::endl;
            this->keyCode = 0;
        }
    }
    
    
    void showkey(const int keyStatusFlag, const QKeyEvent *e) {
        int abc(e->text().toStdString()[0]);
        std::string keyStatus;
        if (keyStatusFlag == 0) {
            std::cout << "keyPressed=0x" << e->key() << " keyCode=0x" << this->keyCode << " text=\'" << e->text().toStdString() << "\' text(hexa)=0x" << std::hex << abc << std::endl;
        } else {
            std::cout << "keyRelease=0x" << e->key() << " keyCode=0x" << this->keyCode << " text=\'" << e->text().toStdString() << "\' text(hexa)=0x" << std::hex << abc << std::endl;
        }  
    }
    
    
    void layout_Work_Update() {
        //                                         ___________________________________________________________________
        //                                        |       |       |        |       |       |        |        |        |
        //   bitcode for _keyboardMods (unint8):  |   0   |   0   |   0    | ALT   | CTRL  | CAPS   | ALTGR  | SHFT   |
        //                                        |_______|_______|________|_______|_______|________|________|________|
        //
        if (this->_keyboardMods & CTRL_MOD) {
            this->_layout = this->_layoutMods[8];  // CTRL
        } else {
            this->_layout = this->_layoutMods[this->_keyboardMods & MASK_ALT];   // set appropriate mod (ALT has no mod)
        }
    }
   
   
    //===================
    //    Characters
    //===================
    void getKeyChar(const QKeyEvent *e) {
        
        switch (keyCode) { 
            case Qt::Key_Eacute      : this->keyCode = 0xE9; break; /*é*/        
            case Qt::Key_Ccedilla    : this->keyCode = 0xE7; break; /*ç*/         
            case Qt::Key_Agrave      : this->keyCode = 0xE0; break; /*à*/         
            case Qt::Key_Ugrave      : this->keyCode = 0xF9; break; /*ù*/
            case Qt::Key_Egrave      : this->keyCode = 0xE8; break; /*è*/
            case Qt::Key_section     : this->keyCode = 0xA7; break; /*§*/
            case Qt::Key_twosuperior : this->keyCode = 0xB2; break; // œ or square
            case Qt::Key_Slash    :
                if (this->_keyboardMods == 0) {
                    keyCode = 0x21;
                    this->flag = this->flag | KBDFLAGS_EXTENDED;
                }
                break;
                
            default: this->keyCode = e->text().toStdString()[0]; break;
        }
        
        //-----------------------------
        //    Keyboard Layout apply
        //-----------------------------
        this->applyCharTable();
    }//=============================================================================
    
    
    //=====================
    //      ShortCuts
    //=====================
    void getCharShortCut() {
        if (this->_keyboardMods & CTRL_MOD) {
            this->_layout = _layoutMods[0]; // noMod
        }

        switch (keyCode) {
            case Qt::Key_A          : this->keyCode = 'a';  break; /*A*/         case Qt::Key_1         : this->keyCode = '1';  break; /*1*/ 
            case Qt::Key_Z          : this->keyCode = 'z';  break; /*Z*/         case Qt::Key_2         : this->keyCode = '2';  break; /*2*/ 
            case Qt::Key_Y          : this->keyCode = 'y';  break; /*Y*/         case Qt::Key_3         : this->keyCode = '3';  break; /*3*/ 
            case Qt::Key_X          : this->keyCode = 'x';  break; /*X*/         case Qt::Key_4         : this->keyCode = '4';  break; /*4*/ 
            case Qt::Key_C          : this->keyCode = 'c';  break; /*C*/         case Qt::Key_5         : this->keyCode = '5';  break; /*5*/ 
            case Qt::Key_V          : this->keyCode = 'v';  break; /*V*/         case Qt::Key_6         : this->keyCode = '6';  break; /*6*/ 
            case Qt::Key_E          : this->keyCode = 'e';  break; /*E*/         case Qt::Key_7         : this->keyCode = '7';  break; /*7*/ 
            case Qt::Key_R          : this->keyCode = 'r';  break; /*R*/         case Qt::Key_8         : this->keyCode = '8';  break; /*8*/ 
            case Qt::Key_T          : this->keyCode = 't';  break; /*T*/         case Qt::Key_9         : this->keyCode = '9';  break; /*9*/ 
            case Qt::Key_U          : this->keyCode = 'u';  break; /*U*/         case Qt::Key_Ampersand : this->keyCode = '&';  break; /*&*/ 
            case Qt::Key_I          : this->keyCode = 'i';  break; /*I*/         case Qt::Key_Asterisk  : this->keyCode = '*';  break; /***/ 
            case Qt::Key_B          : this->keyCode = 'b';  break; /*B*/         case Qt::Key_Underscore: this->keyCode = '_';  break; /*_*/ 
            case Qt::Key_D          : this->keyCode = 'd';  break; /*D*/         case Qt::Key_QuoteDbl  : this->keyCode = '"';  break; /*"*/ 
            case Qt::Key_F          : this->keyCode = 'f';  break; /*F*/         case Qt::Key_Apostrophe: this->keyCode = 39 ;  break; /*'*/ 
            case Qt::Key_G          : this->keyCode = 'g';  break; /*G*/         case Qt::Key_Comma     : this->keyCode = ',';  break; /*,*/ 
            case Qt::Key_H          : this->keyCode = 'h';  break; /*H*/         case Qt::Key_Plus      : this->keyCode = '+';  break; /*+*/ 
            case Qt::Key_J          : this->keyCode = 'j';  break; /*J*/         case Qt::Key_Minus     : this->keyCode = '-';  break; /*-*/ 
            case Qt::Key_K          : this->keyCode = 'k';  break; /*K*/         case Qt::Key_ParenLeft : this->keyCode = '(';  break; /*(*/ 
            case Qt::Key_L          : this->keyCode = 'l';  break; /*L*/         case Qt::Key_ParenRight: this->keyCode = ')';  break; /*)*/ 
            case Qt::Key_M          : this->keyCode = 'm';  break; /*M*/         case Qt::Key_Equal     : this->keyCode = '=';  break; /*=*/ 
            case Qt::Key_N          : this->keyCode = 'n';  break; /*N*/         case Qt::Key_Exclam    : this->keyCode = '!';  break; /*!*/ 
            case Qt::Key_O          : this->keyCode = 'o';  break; /*O*/         case Qt::Key_Colon     : this->keyCode = ':';  break; /*:*/ 
            case Qt::Key_P          : this->keyCode = 'p';  break; /*P*/         case Qt::Key_Dollar    : this->keyCode = '$';  break; /*$*/ 
            case Qt::Key_Q          : this->keyCode = 'q';  break; /*Q*/         case Qt::Key_multiply  : this->keyCode = '*';  break; /***/ 
            case Qt::Key_S          : this->keyCode = 's';  break; /*S*/         case Qt::Key_Space     : this->keyCode = ' ';  break; /* */ 
            case Qt::Key_W          : this->keyCode = 'w';  break; /*W*/         case Qt::Key_section   : this->keyCode = 0xA7; break; /*§*/
            case Qt::Key_Eacute     : this->keyCode = 0xE;  break; /*é*/         //case Qt::Key_         : this->keyCode = '';  break; /**/ 
            case Qt::Key_Ccedilla   : this->keyCode = 0xE7; break; /*ç*/         //case Qt::Key_         : this->keyCode = '';  break; /**/ 
            case Qt::Key_Agrave     : this->keyCode = 0xE0; break; /*à*/         //case Qt::Key_         : this->keyCode = '';  break; /**/ 
            case Qt::Key_Ugrave     : this->keyCode = 0xF9; break; /*ù*/
            case Qt::Key_Egrave     : this->keyCode = 0xE8; break; /*è*/
            case Qt::Key_twosuperior: this->keyCode = 0xB2; break; // œ or square
            case Qt::Key_Slash     :
                if (this->_keyboardMods == 0) {
                    keyCode = 0x21;
                    this->flag = this->flag | KBDFLAGS_EXTENDED;
                }
                break;
            
            default: break;                 
        }
        
        if (this->_keyboardMods & CTRL_MOD) {
            this->_layout = _layoutMods[8];  // ctrl mod
        }
                
        //-----------------------------
        //    Keyboard Layout apply
        //----------------------------
        this->applyCharTable();
    }//=========================================================================================================================
    
    
    //===================
    //   NON CHAR KEYS
    //===================
    //===========================================================================================================================
    void getNonCharKeys() {
        switch (keyCode) {
            
        //------------------------------------------
        //    Not mod neither char keys Extended
        //------------------------------------------
            case Qt::Key_Enter      : this->keyCode = 0x1C; this->flag = this->flag | KBDFLAGS_EXTENDED; break; //  ENTER KP
            case Qt::Key_NumLock    : this->keyCode = 0x45; this->flag = this->flag | KBDFLAGS_EXTENDED; break; //  NUMLOCK
            case Qt::Key_Insert     : this->keyCode = 0x52; this->flag = this->flag | KBDFLAGS_EXTENDED; break; //  INSERT
            case Qt::Key_Delete     : this->keyCode = 0x53; this->flag = this->flag | KBDFLAGS_EXTENDED; break; //  DELETE               
            case Qt::Key_End        : this->keyCode = 0x4F; this->flag = this->flag | KBDFLAGS_EXTENDED; break; //  END
            case Qt::Key_PageDown   : this->keyCode = 0x51; this->flag = this->flag | KBDFLAGS_EXTENDED; break; //  PG DN
            case Qt::Key_PageUp     : this->keyCode = 0x49; this->flag = this->flag | KBDFLAGS_EXTENDED; break; //  PG UP
            case Qt::Key_Up         : this->keyCode = 0x48; this->flag = this->flag | KBDFLAGS_EXTENDED; break; //  U ARROW
            case Qt::Key_Left       : this->keyCode = 0x4B; this->flag = this->flag | KBDFLAGS_EXTENDED; break; //  L ARROW
            case Qt::Key_Down       : this->keyCode = 0x50; this->flag = this->flag | KBDFLAGS_EXTENDED; break; //  D ARROW
            case Qt::Key_Right      : this->keyCode = 0x4D; this->flag = this->flag | KBDFLAGS_EXTENDED; break; //  R ARROW
            case Qt::Key_Meta       : this->keyCode = 0x5c; this->flag = this->flag | KBDFLAGS_EXTENDED; break; //  R WINDOW
            case Qt::Key_Menu       : this->keyCode = 0x5D; this->flag = this->flag | KBDFLAGS_EXTENDED; break; //  MENU APPS

    //-------------------------------------------------------------------------------------------------------------------------
    

        //---------------------------------------------
        //    Not mod neither char keys NO Extended
        //---------------------------------------------
            case Qt::Key_Return     : this->keyCode = 0x1C; break; //  ENTER
            case Qt::Key_Backspace  : this->keyCode = 0x0E; break; //  BKSP
            case Qt::Key_Escape     : this->keyCode = 0x01; break; //  ESCAPE
            case Qt::Key_F1         : this->keyCode = 0x3B; break; //  F1
            case Qt::Key_F2         : this->keyCode = 0x3C; break; //  F2
            case Qt::Key_F3         : this->keyCode = 0x3D; break; //  F3
            case Qt::Key_F4         : this->keyCode = 0x3E; break; //  F4
            case Qt::Key_F5         : this->keyCode = 0x3F; break; //  F5
            case Qt::Key_F6         : this->keyCode = 0x40; break; //  F6
            case Qt::Key_F7         : this->keyCode = 0x41; break; //  F7
            case Qt::Key_F8         : this->keyCode = 0x42; break; //  F8
            case Qt::Key_F9         : this->keyCode = 0x43; break; //  F9
            case Qt::Key_F10        : this->keyCode = 0x44; break; //  F10
            case Qt::Key_F11        : this->keyCode = 0x57; break; //  F11
            case Qt::Key_F12        : this->keyCode = 0x58; break; //  F12
            case Qt::Key_ScrollLock : this->keyCode = 0x46; break; //  SCROLL 
            case Qt::Key_Pause      : this->keyCode = 0xE1; break; //  PAUSE
            case Qt::Key_Tab        : this->keyCode = 0x0F; break; //  TAB
            case Qt::Key_Home       : this->keyCode = 0x47; break; //  HOME

    //----------------------------------------------------------------------
        

            //--------------------------
            //    keyboard modifiers
            //--------------------------
            case Qt::Key_Alt : this->keyCode = 0x38;         //  L ALT
                if (this->flag == 0) {
                    if (this->_keyboardMods & CTRL_MOD) {
                        this->_keyboardMods += ALTGR_MOD;
                        this->_keyboardMods -= CTRL_MOD;
                    } else {
                        this->_keyboardMods += ALT_MOD;
                    }
                } else {
                    if (this->_keyboardMods & ALTGR_MOD) {
                        this->_keyboardMods -= ALT_MOD;
                        this->_keyboardMods += CTRL_MOD;
                    } else {
                        this->_keyboardMods -= ALT_MOD;
                    }
                }
                this->flag = this->flag | KBDFLAGS_EXTENDED;
                this->layout_Work_Update();
                break; 
                
            case Qt::Key_AltGr : this->keyCode = 0x38;       //  R ALT GR
                if (this->flag == 0) {
                    this->_keyboardMods += ALTGR_MOD;
                } else {
                    this->_keyboardMods -= ALTGR_MOD;
                }
                this->flag = this->flag | KBDFLAGS_EXTENDED;
                this->layout_Work_Update();
                break; 
                
            case Qt::Key_Control : this->keyCode = 0x1D;     //  R L CTRL
                if (this->flag == 0) {
                    if (this->_keyboardMods & ALT_MOD) {
                        this->_keyboardMods += ALTGR_MOD;
                        this->_keyboardMods -= ALT_MOD;
                    } else {
                        this->_keyboardMods += CTRL_MOD;
                    }
                } else {
                    if (this->_keyboardMods & ALT_MOD) {
                        this->_keyboardMods -= ALTGR_MOD;
                        this->_keyboardMods += ALT_MOD;
                    } else {
                        this->_keyboardMods -= CTRL_MOD;
                    }
                }
                this->flag = this->flag | KBDFLAGS_EXTENDED;
                this->layout_Work_Update();
                break; 
                
            case Qt::Key_Shift : this->keyCode = 0x36;       // R L SHFT
                if (this->flag == 0) {
                    this->_keyboardMods += SHIFT_MOD;
                } else {
                    this->_keyboardMods -= SHIFT_MOD;
                }
                this->layout_Work_Update();
                break; 
                
            case Qt::Key_CapsLock : this->keyCode = 0x3A;    //  CAPSLOCK 
                if (this->flag == 0) {
                    if ((this->_keyboardMods & CAPSLOCK_MOD) == CAPSLOCK_MOD) {
                        this->_keyboardMods -= CAPSLOCK_MOD;
                    } else {
                        this->_keyboardMods += CAPSLOCK_MOD;
                    }
                    this->layout_Work_Update();
                }                 
                break;
            
            default: break;
        }
    }//==========================================================================================================================
    
    
    //===================
    //   NOT CHAR KEYS
    //===================
    void getDeadKeys() {
        
        switch (keyCode) {
            case Qt::Key_Dead_Circumflex: this->keyCode = '^'; this->_deadKeys = true; break; //  ^ ¨
            case Qt::Key_Dead_Grave     : this->keyCode = '`'; this->_deadKeys = true; break; //  ` grave
            
            default: break;
        }
        
        //-----------------------------
        //    Keyboard Layout apply
        //----------------------------
        this->applyCharTable();
    }//===================================================================================================
    
    
    //==========================
    //  Custom Key NO EXTENDED
    //==========================
    bool getCustomKeysNoExtendedKeylayoutApplied() {
        try {
            this->keyCode = this->_customNoExtendedKeylayoutApplied.at(keyCode);
            
            //-----------------------------
            //    Keyboard Layout apply
            //-----------------------------
            this->applyCharTable();
            return true;
        } catch (const std::exception & e) {
            std::cerr << "Unknown key(" << this->keyCode << ") to Not Extended Custom Key Map applying keylayout." << std::endl;
            return false;
        }
    }//-------------------------------------------------------------------------------------------------------------------------
    
    bool getCustomKeysNoExtended() {
        try {
            this->keyCode = this->_customNoExtended.at(keyCode);
            return true;
        } catch (const std::exception & e) {
            std::cerr << "Unknown key(" << this->keyCode << ") to Not Extended Custom Key Map." << std::endl;
            return false;
        }
    }//========================================================================================================================
    
    
    //==========================
    //   Custom Key EXTENDED
    //==========================
     bool getCustomKeysExtendedKeylayoutApplied() {
        try {
            this->keyCode = this->_customExtendedKeylayoutApplied.at(keyCode);
            
            //-----------------------------
            //    Keyboard Layout apply
            //-----------------------------
            this->applyCharTable();
            this->flag = this->flag | KBDFLAGS_EXTENDED;
            return true;
        } catch (const std::exception & e) {
            std::cerr << "Unknown key(" << this->keyCode << ") to Extended Custom Key Map applying keylayout." << std::endl;
            return false;
        }
    }//---------------------------------------------------------------------------------------------------------------------
    
    bool getCustomKeysExtended() {
        try {
            this->keyCode = this->_customExtended.at(keyCode);
            this->flag = this->flag | KBDFLAGS_EXTENDED;
            return true;
        } catch (const std::exception & e) {
            std::cerr << "Unknown key(" << this->keyCode << ") to Extended Custom Key Map." << std::endl;
            this->keyCode = 0;
            return false;
        }
    }//=====================================================================================================================
    
    
public:
    int keyCode;
    int flag;
    
    //=================//
    //   CONSTRUCTOR   //
    //=====================================================================================================================================
    Qt_RDP_KeyMap(int LCID, int verbose = 0): 
      _verbose(verbose)
    , _keyboardMods(0) 
    , _keylayout_WORK(&keylayout_x80000409)
    , _layoutMods()
    , _layout(&(this->_keylayout_WORK->noMod)) 
    , _customNoExtended()
    , _customExtended()
    , _customNoExtendedKeylayoutApplied()
    , _customExtendedKeylayoutApplied()
    , _deadKeys(false)
    {  
        bool found = false;
        for (uint8_t i = 0 ; i < sizeof(keylayoutsList)/sizeof(keylayoutsList[0]); i++) {
            if (keylayoutsList[i]->LCID == LCID){
                this->_keylayout_WORK = keylayoutsList[i];
                found = true;
                break;
            }
        }
        if (!found){
            std::cout << "Unknown keyboard layout " << LCID << ". Reverting to default (English - United States)" << std::endl;
        }
        
        this->_layoutMods[0] = this->_keylayout_WORK->getnoMod();
        this->_layoutMods[1] = this->_keylayout_WORK->getshift();           
        this->_layoutMods[2] = this->_keylayout_WORK->getaltGr();               
        this->_layoutMods[3] = this->_keylayout_WORK->getshiftAltGr();          
        this->_layoutMods[4] = this->_keylayout_WORK->getcapslock_noMod();      
        this->_layoutMods[5] = this->_keylayout_WORK->getcapslock_shift();      
        this->_layoutMods[6] = this->_keylayout_WORK->getcapslock_altGr();      
        this->_layoutMods[7] = this->_keylayout_WORK->getcapslock_shiftAltGr(); 
        this->_layoutMods[8] = this->_keylayout_WORK->getctrl();
        
        this->_layout = this->_layoutMods[0]; // noMod
    }//====================================================================================================================================
    
    
    ~Qt_RDP_KeyMap() {}
    
    
    void setCustomNoExtendedKeylayoutApplied(int customNoExtended[][2]) {
        if (customNoExtended != nullptr) {
            for (int i = 0 ; i < sizeof(customNoExtended)/sizeof(customNoExtended[0]); i++) {
                if (sizeof(customNoExtended[i])/sizeof(customNoExtended[i][0]) >= 2) {
                   this->_customNoExtendedKeylayoutApplied.emplace(customNoExtended[i][0], customNoExtended[i][1]);
                }
            }
        }
    }
    

    void setCustomExtendedKeylayoutApplied(int customExtended[][2]) {
        if (customExtended != nullptr) {
            for (int i = 0 ; i < sizeof(customExtended)/sizeof(customExtended[0]); i++) {
                if (sizeof(customExtended[i])/sizeof(customExtended[i][0]) >= 2) {
                   this->_customExtendedKeylayoutApplied.emplace(customExtended[i][0], customExtended[i][1]);
                }
            }
        }
    }
    
    
    void setCustomNoExtended(int customNoExtended[][2]) {
        if (customNoExtended != nullptr) {
            for (int i = 0 ; i < sizeof(customNoExtended)/sizeof(customNoExtended[0]); i++) {
                if (sizeof(customNoExtended[i])/sizeof(customNoExtended[i][0]) >= 2) {
                   this->_customNoExtended.emplace(customNoExtended[i][0], customNoExtended[i][1]);
                }
            }
        }
    }
    

    void setCustomExtended(int customExtended[][2]) {
        if (customExtended != nullptr) {
            for (int i = 0 ; i < sizeof(customExtended)/sizeof(customExtended[0]); i++) {
                if (sizeof(customExtended[i])/sizeof(customExtended[i][0]) >= 2) {
                   this->_customExtended.emplace(customExtended[i][0], customExtended[i][1]);
                }
            }
        }
    }
    
    
    void keyQtEvent(const int keyStatusFlag, const QKeyEvent *e) { 
        this->flag = keyStatusFlag;
        this->keyCode = e->key(); 
        this->_deadKeys = false;
        
        if (this->keyCode != 0) {
            
            if (this->keyCode < MAX_CHAR_QT_KEY_VALUE ) {
                if (this->_keyboardMods & CTRL_MOD || this->_keyboardMods == ALT_MOD) {
                    
                    //--------------------------------------
                    //      Char ShortCuts CTRL or ALT
                    //--------------------------------------
                    this->getCharShortCut();
                } else {
                    
                    //--------------------------------------
                    //             CHARACTERS
                    //--------------------------------------
                    this->getKeyChar(e);
                }
            } else {
            
                if (this->keyCode > MIN_NON_CHAR_QT_KEY_VALUE) {
                    if (this->keyCode < MAX_DEAD_QT_KEY_VALUE && this->keyCode > MIN_DEAD_QT_KEY_VALUE) {
                        
                        //--------------------------------------
                        //              DEAD KEYS
                        //--------------------------------------
                        this->getDeadKeys();
                    } else {
                        
                        //--------------------------------------
                        //            NON CHAR KEYS
                        //--------------------------------------
                        this->getNonCharKeys();
                    }
                } else {
                    
                    //--------------------------
                    //  Custom Key NO EXTENDED
                    //--------------------------
                    if (!this->getCustomKeysNoExtended()) {
                        if (!this->getCustomKeysNoExtendedKeylayoutApplied()) {
                            
                            //--------------------------
                            //   Custom Key EXTENDED
                            //--------------------------
                            if (!this->getCustomKeysExtended()) {
                                this->getCustomKeysExtendedKeylayoutApplied();
                            }
                        }
                    }
                }
            }
        }
        
        
        if (this->_verbose > 0) {
            this->showkey(keyStatusFlag, e);
        }
    }
                
    
};
    
#endif