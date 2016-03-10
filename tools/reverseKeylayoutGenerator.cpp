/*
 * 
 */


#include <iostream>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>
#include "../src/keyboard/keylayouts.hpp"
//#include "keymap2.hpp"
#define SIZE 128


void tabToReversedMap(const Keylayout::KeyLayout_t & read, std::ofstream & fichier, std::string name);
bool keyLayout1IsInferiorThanKeylayout2(const Keylayout & k1, const Keylayout & k2);

static const Keylayout * keylayouts[] = { &keylayout_x00000405, &keylayout_x00000406, &keylayout_x00000407
                                            , &keylayout_x00000408, &keylayout_x00000409, &keylayout_x0000040a
                                            , &keylayout_x0000040b, &keylayout_x0000040c, &keylayout_x0000040f
                                            , &keylayout_x00000410, &keylayout_x00000413, &keylayout_x00000414
                                            , &keylayout_x00000415, &keylayout_x00000416, &keylayout_x00000418
                                            , &keylayout_x00000419, &keylayout_x0000041a, &keylayout_x0000041b
                                            , &keylayout_x0000041d, &keylayout_x0000041f, &keylayout_x00000422
                                            , &keylayout_x00000424, &keylayout_x00000425, &keylayout_x00000426
                                            , &keylayout_x00000427, &keylayout_x0000042f, &keylayout_x00000438
                                            , &keylayout_x0000043a, &keylayout_x0000043b, &keylayout_x0000043f
                                            , &keylayout_x00000440, &keylayout_x00000444, &keylayout_x00000450
                                            , &keylayout_x00000452, &keylayout_x0000046e, &keylayout_x00000481
                                            , &keylayout_x00000807, &keylayout_x00000809, &keylayout_x0000080a
                                            , &keylayout_x0000080c, &keylayout_x00000813, &keylayout_x00000816
                                            , &keylayout_x0000081a, &keylayout_x0000083b, &keylayout_x00000843
                                            , &keylayout_x0000085d, &keylayout_x00000c0c, &keylayout_x00000c1a
                                            , &keylayout_x00001009, &keylayout_x0000100c, &keylayout_x0000201a
                                            , &keylayout_x00010402, &keylayout_x00010405, &keylayout_x00001809
                                            , &keylayout_x00010407, &keylayout_x00010408, &keylayout_x0001040a
                                            , &keylayout_x0001040e, &keylayout_x00010409, &keylayout_x00010410
                                            , &keylayout_x00010415, &keylayout_x00010416, &keylayout_x00010419
                                            , &keylayout_x0001041b, &keylayout_x0001041f, &keylayout_x00010426
                                            , &keylayout_x00010427, &keylayout_x0001043a, &keylayout_x0001043b
                                            , &keylayout_x0001080c, &keylayout_x0001083b, &keylayout_x00011009
                                            , &keylayout_x00011809, &keylayout_x00020405, &keylayout_x00020408
                                            , &keylayout_x00020409, &keylayout_x0002083b, &keylayout_x00030402
                                            , &keylayout_x00030408, &keylayout_x00030409, &keylayout_x00040408
                                            , &keylayout_x00040409, &keylayout_x00050408, &keylayout_x00060408
                                            };  
                                            
static int over(0);
    
    
int main () {
    
    /*
    std::vector<const Keylayout *> vecKeylayouts;
    
    vecKeylayouts.push_back(keylayouts[0]);
    
    for(int i = 1; i < 84; i++) {
        bool added(false);
        int size(vecKeylayouts.size());
        for (int j = 0; j < size; j++) {
            if (keyLayout1IsInferiorThanKeylayout2(*keylayouts[i], *vecKeylayouts[j])) {
                vecKeylayouts.insert(vecKeylayouts.begin() + j, keylayouts[i]);
                added = true;
            }
            if (added) {
                break;
            }
        }
        
        if (!added) {
            vecKeylayouts.push_back(keylayouts[i]);
        }
    }
    
    for(int i = 0; i < 84; i++) {
        std::cout << vecKeylayouts[i]->locale_name << std::endl;
    }
    
    std:: cout << "static const Keylayout * keylayouts[] = {";            
    for(int i = 0; i < 84; i++) {
        
        if (i != 0) {
         std::cout << " ,";   
        }

        std::cout << "&keylayout_x800";
        
        if (vecKeylayouts[i]->LCID < 0x00010000) {
            std::cout << "0";
        }
        if (vecKeylayouts[i]->LCID < 0x00001000) {
            std::cout << "0";
        }
        
        std::cout << std::hex << vecKeylayouts[i]->LCID;
        
        if ((i+1)%3 == 0) {
            std::cout << std::endl << "                                       ";
        }
    }
    */
        
    for(int i = 0; i < 84; i++) {
        const Keylayout *  keylayout_WORK(keylayouts[i]); 
             
        
        std::string loc_name(keylayout_WORK->locale_name);
        std::replace( loc_name.begin(), loc_name.end(), '.', '_');
        std::replace( loc_name.begin(), loc_name.end(), '-', '_');
        boost::to_upper(loc_name);
        
         std::cout << loc_name;
        
            for (int j = 0; j < (25 - loc_name.size()); j++) {
                std::cout << " ";
            }
        
        
        std::cout << " = 0x" << std::hex << keylayout_WORK->LCID +0x80000000 << ", \t";
        
        if ((i)%3 == 2 && i != 0) {
            std::cout << std::endl;   
        }
        
        
        int LCID(keylayout_WORK->LCID);
        int LCIDreverse(LCID+0x80000000);
        
        std::stringstream ss;
        ss << std::hex << LCIDreverse;
        std::string name = ss.str();
        std::ofstream fichier(("src/keyboard/reversed_keymaps/keylayout_x"+name+".hpp").c_str(), std::ios::out | std::ios::trunc);
        
        
        if(fichier) {

            fichier << "#ifndef _REDEMPTION_KEYBOARD_KEYLAYOUT_R_X"<<std::hex<<LCIDreverse<<"_HPP_" << std::endl;
            fichier << "#define _REDEMPTION_KEYBOARD_KEYLAYOUT_R_X"<<std::hex<<LCIDreverse<<"_HPP_" << std::endl<< std::endl;
            
            fichier << "#include \"keylayout_r.hpp\"" << std::endl<< std::endl;
            
            fichier << "namespace x"<< std::hex<<LCIDreverse << "{ " << std::endl<< std::endl;
            
            fichier << "const static int LCID = 0x"<< std::hex<< LCID <<";"<< std::endl<< std::endl;

            fichier << "const static char * const locale_name = \""<<keylayout_WORK->locale_name<<"\";"<< std::endl<< std::endl;

            tabToReversedMap(keylayout_WORK->noMod,               fichier,  "noMod");
            tabToReversedMap(keylayout_WORK->shift,               fichier,  "shift");
            tabToReversedMap(keylayout_WORK->altGr,               fichier,  "altGr");
            tabToReversedMap(keylayout_WORK->shiftAltGr,          fichier,  "shiftAltGr");
            tabToReversedMap(keylayout_WORK->capslock_noMod,      fichier,  "capslock_noMod");
            tabToReversedMap(keylayout_WORK->capslock_shift,      fichier,  "capslock_shift");
            tabToReversedMap(keylayout_WORK->capslock_altGr,      fichier,  "capslock_altGr");
            tabToReversedMap(keylayout_WORK->capslock_shiftAltGr, fichier,  "capslock_shiftAltGr");
            tabToReversedMap(keylayout_WORK->ctrl,                fichier,  "ctrl");
            
            fichier << "const Keylayout_r::KeyLayoutMap_t deadkeys" << std::endl;
            fichier << "{" << std::endl;
            for (int i = 0; i < keylayout_WORK->nbDeadkeys; i++) {
                int deadCode(keylayout_WORK->deadkeys[i].extendedKeyCode);
                fichier << "\t{ 0x" << std::hex << keylayout_WORK->deadkeys[i].uchar << ", 0x" << deadCode << "}," << std::endl;
                int nbSecondDK(keylayout_WORK->deadkeys[i].nbSecondKeys);
                for (int j = 0; j < nbSecondDK; j++) {
                    fichier << "\t{ 0x" << std::hex << keylayout_WORK->deadkeys[i].secondKeys[j].secondKey << ", 0x" << keylayout_WORK->deadkeys[i].secondKeys[j].modifiedKey << "}," << std::endl;
                }
            }
            fichier << "};" << std::endl;
            fichier << std::endl << std::endl;

            int nbDeadKeys(keylayout_WORK->nbDeadkeys);
            fichier << "const static uint8_t nbDeadkeys = " << std::dec << nbDeadKeys << ";" << std::endl << std::endl;
            
            fichier << "}" << std::endl << std::endl;
            
            fichier << "static const Keylayout_r keylayout_x"<< std::hex<<LCIDreverse <<"( x"<< std::hex<<LCIDreverse <<"::LCID" << std::endl;
            fichier << "                                 , x"<< std::hex<<LCIDreverse <<"::locale_name" << std::endl;
            fichier << "                                 , x"<< std::hex<<LCIDreverse <<"::noMod" << std::endl;
            fichier << "                                 , x"<< std::hex<<LCIDreverse <<"::shift" << std::endl;
            fichier << "                                 , x"<< std::hex<<LCIDreverse <<"::altGr" << std::endl;
            fichier << "                                 , x"<< std::hex<<LCIDreverse <<"::shiftAltGr" << std::endl;
            fichier << "                                 , x"<< std::hex<<LCIDreverse <<"::ctrl" << std::endl;
            fichier << "                                 , x"<< std::hex<<LCIDreverse <<"::capslock_noMod" << std::endl;
            fichier << "                                 , x"<< std::hex<<LCIDreverse <<"::capslock_shift" << std::endl;
            fichier << "                                 , x"<< std::hex<<LCIDreverse <<"::capslock_altGr" << std::endl;
            fichier << "                                 , x"<< std::hex<<LCIDreverse <<"::capslock_shiftAltGr" << std::endl;
            fichier << "                                 , x"<< std::hex<<LCIDreverse <<"::deadkeys" << std::endl;
            fichier << "                                 , x"<< std::hex<<LCIDreverse <<"::nbDeadkeys" << std::endl << std::endl;
            fichier << ");" << std::endl << std::endl;

            fichier << "#endif" << std::endl;
            
            fichier.close();  

        } else {

            std::cerr << "Error file" << std::endl;

        }
    }
    
    std::cout << " };" << std::endl; 
    
    std::cout << std::endl << std::dec << "missing data = " << over << std::endl;
    
    return 0;
}


bool keyLayout1IsInferiorThanKeylayout2(const Keylayout & k1, const Keylayout & k2) {
    std::string k1Name(k1.locale_name);
    std::string k2Name(k2.locale_name);
    int length(0);
    bool k1IsShorter(false);
    if (k1Name.length() < k2Name.length()) {
        length = k1Name.length();
        k1IsShorter = true;
    } else {
        length = k2Name.length();
    }
    
    for (int i = 0; i < length; i++) {
        char k1char(k1Name[i]);
        char k2char(k2Name[i]);
        if (k1char < k2char) {
            return true;
        }
        if (k1char > k2char) {
            return false;
        }
    }
    
    return k1IsShorter;
}



void tabToReversedMap(const Keylayout::KeyLayout_t & read, std::ofstream & fichier, std::string name) {
    fichier << "const Keylayout_r::KeyLayoutMap_t "<< name << std::endl;
    fichier << "{" << std::endl;
    for(int i = 0; i < SIZE; i++) {
        if (read[i] != 0){
            if (read[i] < 16) {
                fichier << "\t{ " << "0x000" << read[i] << ", 0x" << i << " }," << std::endl;
            } else if (read[i] < 256){
                fichier << "\t{ " << "0x00" << read[i] << ", 0x" << i << " }," << std::endl;
            } else if (read[i] < 256*16) {
                fichier << "\t{ " << "0x0" << read[i] << ", 0x" << i << " }," << std::endl;
            } else if (read[i] < 256*256) {
                fichier << "\t{ " << "0x" << read[i] << ", 0x" << i << " }," << std::endl;
            } else {
                over++;
            }
        }
    }
    fichier << "};" << std::endl;
    
    fichier << std::endl << std::endl;
}

