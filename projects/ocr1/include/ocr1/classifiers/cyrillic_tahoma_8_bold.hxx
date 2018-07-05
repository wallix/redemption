if (count < 35) {
  if (count < 20) {
    if (count < 13) {
      if (count < 8) {
        if (count == 4) {
          if (width == 2) {
            return ".";
          } else if (width == 3) {
            return "`";
          } else if (width == 4) {
            return "-";
          } else /* NOLINT */ {}
        } else if (count == 6) {
          return "'";
        } else if (count == 7) {
          if (width == 2) {
            return ",";
          } else if (width == 7) {
            return "_";
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      } else {
        if (count == 8) {
          if (width == 2) {
            return ":";
          } else if (width == 4) {
            return "°";
          } else /* NOLINT */ {}
        } else if (count == 10) {
          return "—";
        } else if (count == 11) {
          if (width == 2) {
            return ";";
          } else if (width == 5) {
            if (height == 11) {
              if (pixel(0) == 1) {
                return "\\";
              } else if (pixel(0) == 0) {
                return "/";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else if (count == 12) {
          if (width == 2) {
            return "\x1";
          } else if (width == 8) {
            return "~";
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      }
    } else {
      if (count < 16) {
        if (count == 13) {
          if (width == 7) {
            if (height == 7) {
              if (pixel(0) == 1) {
                return ">";
              } else if (pixel(0) == 0) {
                if (pixel(3) == 1) {
                  return "+";
                } else if (pixel(3) == 0) {
                  return "<";
                } else /* NOLINT */ {}
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else if (count == 14) {
          if (width == 2) {
            if (height == 8) {
              if (pixel(2) == 1) {
                return "!";
              } else if (pixel(2) == 0) {
                return "i";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else if (width == 7) {
            return "=";
          } else /* NOLINT */ {}
        } else if (count == 15) {
          if (width == 4) {
            return "r";
          } else if (width == 5) {
            return "г";
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      } else {
        if (count == 16) {
          if (width == 2) {
            return "\x2";
          } else if (width == 4) {
            return "î";
          } else if (width == 5) {
            return "c";
          } else if (width == 6) {
            if (height == 5) {
              return "*";
            } else if (height == 6) {
              if (pixel(1) == 1) {
                return "т";
              } else if (pixel(1) == 0) {
                return "¤";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else if (count == 17) {
          return "з";
        } else if (count == 18) {
          if (width == 2) {
            return "l";
          } else if (width == 5) {
            if (height == 6) {
              if (pixel(1) == 1) {
                if (pixel(4) == 1) {
                  return "z";
                } else if (pixel(4) == 0) {
                  return "э";
                } else /* NOLINT */ {}
              } else if (pixel(1) == 0) {
                return "и";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else if (count == 19) {
          if (width == 3) {
            return "j";
          } else if (width == 4) {
            if (height == 8) {
              if (pixel(0) == 1) {
                return "t";
              } else if (pixel(0) == 0) {
                return "1";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else if (width == 5) {
            if (height == 8) {
              if (pixel(2) == 1) {
                if (pixel(0) == 1) {
                  return "Г";
                } else if (pixel(0) == 0) {
                  return "ç";
                } else /* NOLINT */ {}
              } else if (pixel(2) == 0) {
                return "L";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      }
    }
  } else {
    if (count < 27) {
      if (count < 23) {
        if (count == 20) {
          if (width == 4) {
            if (height == 8) {
              return "I";
            } else if (height == 9) {
              return "f";
            } else /* NOLINT */ {}
          } else if (width == 5) {
            if (height == 6) {
              return "s";
            } else if (height == 8) {
              return "J";
            } else /* NOLINT */ {}
          } else if (width == 6) {
            if (height == 6) {
              return "v";
            } else if (height == 8) {
              if (pixel(8) == 1) {
                return "T";
              } else if (pixel(8) == 0) {
                return "7";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else if (count == 21) {
          return "ч";
        } else if (count == 22) {
          if (width == 4) {
            if (height == 11) {
              if (pixel(0) == 1) {
                return ")";
              } else if (pixel(0) == 0) {
                return "(";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else if (width == 5) {
            return "F";
          } else if (width == 6) {
            if (height == 6) {
              return "ь";
            } else if (height == 8) {
              if (pixel(1) == 1) {
                return "C";
              } else if (pixel(1) == 0) {
                return "4";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      } else {
        if (count == 23) {
          if (width == 5) {
            return "й";
          } else if (width == 6) {
            return "e";
          } else /* NOLINT */ {}
        } else if (count == 24) {
          if (width == 6) {
            if (height == 6) {
              if (pixel(6) == 1) {
                if (pixel(0) == 1) {
                  if (pixel(9) == 1) {
                    return "к";
                  } else if (pixel(9) == 0) {
                    return "x";
                  } else /* NOLINT */ {}
                } else if (pixel(0) == 0) {
                  return "o";
                } else /* NOLINT */ {}
              } else if (pixel(6) == 0) {
                return "a";
              } else /* NOLINT */ {}
            } else if (height == 8) {
              if (pixel(0) == 1) {
                if (pixel(2) == 1) {
                  return "З";
                } else if (pixel(2) == 0) {
                  if (pixel(37) == 1) {
                    return "y";
                  } else if (pixel(37) == 0) {
                    return "Y";
                  } else /* NOLINT */ {}
                } else /* NOLINT */ {}
              } else if (pixel(0) == 0) {
                return "2";
              } else /* NOLINT */ {}
            } else if (height == 10) {
              return "Ï";
            } else /* NOLINT */ {}
          } else if (width == 8) {
            return "ъ";
          } else /* NOLINT */ {}
        } else if (count == 25) {
          if (width == 5) {
            return "E";
          } else if (width == 6) {
            if (height == 6) {
              if (pixel(0) == 1) {
                if (pixel(2) == 1) {
                  return "n";
                } else if (pixel(2) == 0) {
                  return "u";
                } else /* NOLINT */ {}
              } else if (pixel(0) == 0) {
                return "л";
              } else /* NOLINT */ {}
            } else if (height == 8) {
              if (pixel(0) == 1) {
                return "Э";
              } else if (pixel(0) == 0) {
                if (pixel(1) == 1) {
                  if (pixel(5) == 1) {
                    return "5";
                  } else if (pixel(5) == 0) {
                    return "3";
                  } else /* NOLINT */ {}
                } else if (pixel(1) == 0) {
                  return "£";
                } else /* NOLINT */ {}
              } else /* NOLINT */ {}
            } else if (height == 11) {
              return "}";
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else if (count == 26) {
          if (width == 4) {
            if (height == 11) {
              if (pixel(4) == 1) {
                return "[";
              } else if (pixel(4) == 0) {
                return "]";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else if (width == 6) {
            if (height == 6) {
              if (pixel(0) == 1) {
                if (pixel(2) == 1) {
                  return "п";
                } else if (pixel(2) == 0) {
                  return "н";
                } else /* NOLINT */ {}
              } else if (pixel(0) == 0) {
                return "я";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else if (width == 7) {
            return "У";
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      }
    } else {
      if (count < 31) {
        if (count == 27) {
          if (width == 6) {
            if (height == 6) {
              return "в";
            } else if (height == 8) {
              return "ë";
            } else if (height == 9) {
              if (pixel(1) == 1) {
                return "è";
              } else if (pixel(1) == 0) {
                return "é";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else if (count == 28) {
          if (width == 6) {
            if (height == 8) {
              if (pixel(8) == 1) {
                return "N";
              } else if (pixel(8) == 0) {
                if (pixel(4) == 1) {
                  if (pixel(5) == 1) {
                    if (pixel(13) == 1) {
                      if (pixel(3) == 1) {
                        return "S";
                      } else if (pixel(3) == 0) {
                        if (pixel(0) == 1) {
                          if (pixel(6) == 1) {
                            if (pixel(12) == 1) {
                              return "V";
                            } else if (pixel(12) == 0) {
                              return "X";
                            } else /* NOLINT */ {}
                          } else if (pixel(6) == 0) {
                            return "ö";
                          } else /* NOLINT */ {}
                        } else if (pixel(0) == 0) {
                          return "ä";
                        } else /* NOLINT */ {}
                      } else /* NOLINT */ {}
                    } else if (pixel(13) == 0) {
                      return "Z";
                    } else /* NOLINT */ {}
                  } else if (pixel(5) == 0) {
                    return "P";
                  } else /* NOLINT */ {}
                } else if (pixel(4) == 0) {
                  return "Ь";
                } else /* NOLINT */ {}
              } else /* NOLINT */ {}
            } else if (height == 9) {
              return "à";
            } else /* NOLINT */ {}
          } else if (width == 7) {
            return "Ч";
          } else /* NOLINT */ {}
        } else if (count == 29) {
          if (width == 5) {
            return "Ë";
          } else if (width == 6) {
            if (height == 8) {
              if (pixel(3) == 1) {
                if (pixel(10) == 1) {
                  return "9";
                } else if (pixel(10) == 0) {
                  return "6";
                } else /* NOLINT */ {}
              } else if (pixel(3) == 0) {
                return "ü";
              } else /* NOLINT */ {}
            } else if (height == 9) {
              return "ù";
            } else /* NOLINT */ {}
          } else if (width == 7) {
            return "€";
          } else /* NOLINT */ {}
        } else if (count == 30) {
          if (width == 6) {
            if (height == 8) {
              if (pixel(0) == 1) {
                if (pixel(2) == 1) {
                  return "p";
                } else if (pixel(2) == 0) {
                  if (pixel(9) == 1) {
                    return "K";
                  } else if (pixel(9) == 0) {
                    return "µ";
                  } else /* NOLINT */ {}
                } else /* NOLINT */ {}
              } else if (pixel(0) == 0) {
                return "q";
              } else /* NOLINT */ {}
            } else if (height == 9) {
              if (pixel(0) == 1) {
                return "k";
              } else if (pixel(0) == 0) {
                return "ê";
              } else /* NOLINT */ {}
            } else if (height == 11) {
              return "$";
            } else /* NOLINT */ {}
          } else if (width == 7) {
            return "м";
          } else if (width == 8) {
            return "Ъ";
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      } else {
        if (count == 31) {
          if (width == 6) {
            if (height == 8) {
              return "Б";
            } else if (height == 9) {
              if (pixel(0) == 1) {
                return "h";
              } else if (pixel(0) == 0) {
                if (pixel(24) == 1) {
                  return "ô";
                } else if (pixel(24) == 0) {
                  return "â";
                } else /* NOLINT */ {}
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else if (width == 7) {
            if (height == 8) {
              if (pixel(0) == 1) {
                if (pixel(1) == 1) {
                  return "ц";
                } else if (pixel(1) == 0) {
                  return "И";
                } else /* NOLINT */ {}
              } else if (pixel(0) == 0) {
                return "G";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else if (count == 32) {
          if (width == 6) {
            if (height == 8) {
              if (pixel(0) == 1) {
                return "К";
              } else if (pixel(0) == 0) {
                if (pixel(5) == 1) {
                  return "g";
                } else if (pixel(5) == 0) {
                  if (pixel(18) == 1) {
                    return "0";
                  } else if (pixel(18) == 0) {
                    return "8";
                  } else /* NOLINT */ {}
                } else /* NOLINT */ {}
              } else /* NOLINT */ {}
            } else if (height == 9) {
              if (pixel(0) == 1) {
                return "b";
              } else if (pixel(0) == 0) {
                if (pixel(2) == 1) {
                  return "û";
                } else if (pixel(2) == 0) {
                  return "d";
                } else /* NOLINT */ {}
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else if (count == 33) {
          if (width == 6) {
            return "б";
          } else if (width == 7) {
            if (height == 8) {
              if (pixel(0) == 1) {
                return "U";
              } else if (pixel(0) == 0) {
                return "A";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else if (count == 34) {
          if (width == 6) {
            if (height == 9) {
              return "fi";
            } else if (height == 10) {
              return "§";
            } else /* NOLINT */ {}
          } else if (width == 7) {
            if (height == 8) {
              if (pixel(1) == 1) {
                if (pixel(6) == 1) {
                  return "Л";
                } else if (pixel(6) == 0) {
                  if (pixel(0) == 1) {
                    if (pixel(18) == 1) {
                      return "R";
                    } else if (pixel(18) == 0) {
                      return "D";
                    } else /* NOLINT */ {}
                  } else if (pixel(0) == 0) {
                    return "O";
                  } else /* NOLINT */ {}
                } else /* NOLINT */ {}
              } else if (pixel(1) == 0) {
                return "Я";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else if (width == 8) {
            return "w";
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      }
    }
  }
} else {
  if (count < 54) {
    if (count < 43) {
      if (count < 38) {
        if (count == 35) {
          if (width == 6) {
            return "B";
          } else if (width == 7) {
            if (height == 8) {
              if (pixel(2) == 1) {
                return "П";
              } else if (pixel(2) == 0) {
                return "H";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else if (width == 8) {
            if (height == 8) {
              return "&";
            } else if (height == 9) {
              return "ît";
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else if (count == 36) {
          if (width == 7) {
            return "Й";
          } else if (width == 10) {
            return "ж";
          } else /* NOLINT */ {}
        } else if (count == 37) {
          if (width == 7) {
            if (height == 10) {
              if (pixel(14) == 1) {
                return "Ü";
              } else if (pixel(14) == 0) {
                return "Ä";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else if (width == 8) {
            return "д";
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      } else {
        if (count == 38) {
          if (width == 6) {
            return "fl";
          } else if (width == 7) {
            return "Ö";
          } else if (width == 8) {
            if (height == 6) {
              return "ш";
            } else if (height == 9) {
              return "#";
            } else /* NOLINT */ {}
          } else if (width == 10) {
            if (height == 6) {
              if (pixel(2) == 1) {
                return "m";
              } else if (pixel(2) == 0) {
                return "ю";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else if (count == 39) {
          if (width == 7) {
            return "Q";
          } else if (width == 8) {
            return "ft";
          } else if (width == 9) {
            return "M";
          } else /* NOLINT */ {}
        } else if (count == 40) {
          if (width == 7) {
            return "Â";
          } else if (width == 8) {
            if (height == 8) {
              return "Ф";
            } else if (height == 9) {
              return "ff";
            } else if (height == 10) {
              return "Ц";
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else if (count == 42) {
          return "@";
        } else /* NOLINT */ {}
      }
    } else {
      if (count < 48) {
        if (count == 43) {
          return "щ";
        } else if (count == 44) {
          return "%";
        } else if (count == 46) {
          if (width == 8) {
            return "ф";
          } else if (width == 10) {
            return "W";
          } else /* NOLINT */ {}
        } else if (count == 47) {
          return "Д";
        } else /* NOLINT */ {}
      } else {
        if (count == 48) {
          return "Ж";
        } else if (count == 50) {
          return "fk";
        } else if (count == 51) {
          return "fh";
        } else if (count == 52) {
          if (width == 10) {
            if (height == 8) {
              return "Ш";
            } else if (height == 9) {
              return "fb";
            } else /* NOLINT */ {}
          } else if (width == 11) {
            return "Ю";
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      }
    }
  } else {
    if (count < 72) {
      if (count < 59) {
        if (count == 54) {
          return "ffi";
        } else if (count == 57) {
          return "Щ";
        } else if (count == 58) {
          return "ffl";
        } else /* NOLINT */ {}
      } else {
        if (count == 59) {
          return "fft";
        } else if (count == 60) {
          return "fff";
        } else if (count == 70) {
          return "ffk";
        } else if (count == 71) {
          return "ffh";
        } else /* NOLINT */ {}
      }
    } else {
      if (count < 80) {
        if (count == 72) {
          return "ffb";
        } else if (count == 74) {
          return "fffi";
        } else if (count == 78) {
          return "fffl";
        } else if (count == 79) {
          return "ffft";
        } else /* NOLINT */ {}
      } else {
        if (count == 80) {
          return "ffff";
        } else if (count == 90) {
          return "fffk";
        } else if (count == 91) {
          return "fffh";
        } else if (count == 92) {
          return "fffb";
        } else /* NOLINT */ {}
      }
    }
  }
}
