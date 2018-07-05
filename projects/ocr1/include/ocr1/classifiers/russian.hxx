if (count < 19) {
  if (count < 10) {
    if (count < 6) {
      if (count == 2) {
        return ".";
      } else /* NOLINT */ if (count == 3) {
        if (width == 1) {
          return "'";
        } else /* NOLINT */ if (width == 2) {
          return ",";
        } else /* NOLINT */ if (width == 3) {
          return "-";
        } else /* NOLINT */ {}
      } else /* NOLINT */ if (count == 4) {
        return ":";
      } else /* NOLINT */ if (count == 5) {
        if (width == 2) {
          return ";";
        } else /* NOLINT */ if (width == 5) {
          return "_";
        } else /* NOLINT */ {}
      } else /* NOLINT */ {}
    } else /* NOLINT */ {
      if (count == 6) {
        if (width == 1) {
          return "\x1";
        } else /* NOLINT */ if (width == 6) {
          return "~";
        } else /* NOLINT */ {}
      } else /* NOLINT */ if (count == 7) {
        return "i";
      } else /* NOLINT */ if (count == 8) {
        return "!";
      } else /* NOLINT */ if (count == 9) {
        if (width == 1) {
          return "\x2";
        } else /* NOLINT */ if (width == 4) {
          return "г";
        } else /* NOLINT */ if (width == 5) {
          return "*";
        } else /* NOLINT */ {}
      } else /* NOLINT */ {}
    }
  } else /* NOLINT */ {
    if (count < 14) {
      if (count == 10) {
        if (width == 4) {
          return "c";
        } else /* NOLINT */ if (width == 5) {
          if (height == 6) {
            if (pixel(1) == 1) {
              return "т";
            } else /* NOLINT */ if (pixel(1) == 0) {
              return "х";
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      } else /* NOLINT */ if (count == 11) {
        if (width == 3) {
          if (height == 11) {
            if (pixel(0) == 1) {
              return ")";
            } else /* NOLINT */ if (pixel(0) == 0) {
              return "(";
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (width == 4) {
          return "з";
        } else /* NOLINT */ {}
      } else /* NOLINT */ if (count == 12) {
        if (width == 4) {
          if (height == 6) {
            if (pixel(1) == 1) {
              return "э";
            } else /* NOLINT */ if (pixel(1) == 0) {
              return "к";
            } else /* NOLINT */ {}
          } else /* NOLINT */ if (height == 9) {
            return "7";
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (width == 5) {
          return "ч";
        } else /* NOLINT */ if (width == 12) {
          return "—";
        } else /* NOLINT */ {}
      } else /* NOLINT */ if (count == 13) {
        if (width < 4) {
          if (width == 2) {
            if (height == 11) {
              if (pixel(2) == 1) {
                return "[";
              } else /* NOLINT */ if (pixel(2) == 0) {
                return "]";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ if (width == 3) {
            return "I";
          } else /* NOLINT */ {}
        } else /* NOLINT */ {
          if (width == 4) {
            return "e";
          } else /* NOLINT */ if (width == 5) {
            return "Г";
          } else /* NOLINT */ if (width == 7) {
            return "ѵ";
          } else /* NOLINT */ {}
        }
      } else /* NOLINT */ {}
    } else /* NOLINT */ {
      if (count < 16) {
        if (count == 14) {
          if (width == 4) {
            if (height == 6) {
              if (pixel(0) == 1) {
                return "a";
              } else /* NOLINT */ if (pixel(0) == 0) {
                return "я";
              } else /* NOLINT */ {}
            } else /* NOLINT */ if (height == 9) {
              if (pixel(17) == 1) {
                return "3";
              } else /* NOLINT */ if (pixel(17) == 0) {
                return "2";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ if (width == 5) {
            if (height == 6) {
              if (pixel(0) == 1) {
                return "ь";
              } else /* NOLINT */ if (pixel(0) == 0) {
                return "o";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ if (width == 6) {
            return "л";
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (count == 15) {
          if (width == 4) {
            if (height == 6) {
              return "в";
            } else /* NOLINT */ if (height == 8) {
              return "ё";
            } else /* NOLINT */ {}
          } else /* NOLINT */ if (width == 5) {
            if (height == 6) {
              if (pixel(1) == 1) {
                return "п";
              } else /* NOLINT */ if (pixel(1) == 0) {
                return "н";
              } else /* NOLINT */ {}
            } else /* NOLINT */ if (height == 9) {
              return "1";
            } else /* NOLINT */ {}
          } else /* NOLINT */ if (width == 6) {
            return "C";
          } else /* NOLINT */ if (width == 7) {
            if (height == 9) {
              if (pixel(1) == 1) {
                return "T";
              } else /* NOLINT */ if (pixel(1) == 0) {
                return "У";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      } else /* NOLINT */ {
        if (count == 16) {
          if (width == 4) {
            return "5";
          } else /* NOLINT */ if (width == 5) {
            return "и";
          } else /* NOLINT */ if (width == 6) {
            return "у";
          } else /* NOLINT */ if (width == 7) {
            return "ъ";
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (count == 17) {
          if (width == 4) {
            return "6";
          } else /* NOLINT */ if (width == 5) {
            return "З";
          } else /* NOLINT */ if (width == 7) {
            return "X";
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (count == 18) {
          if (width == 4) {
            if (height == 9) {
              if (pixel(0) == 1) {
                return "E";
              } else /* NOLINT */ if (pixel(0) == 0) {
                if (pixel(16) == 1) {
                  if (pixel(20) == 1) {
                    return "0";
                  } else /* NOLINT */ if (pixel(20) == 0) {
                    return "9";
                  } else /* NOLINT */ {}
                } else /* NOLINT */ if (pixel(16) == 0) {
                  return "8";
                } else /* NOLINT */ {}
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ if (width == 5) {
            if (height == 9) {
              if (pixel(0) == 1) {
                return "Ь";
              } else /* NOLINT */ if (pixel(0) == 0) {
                return "4";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ if (width == 6) {
            if (height == 8) {
              return "ц";
            } else /* NOLINT */ if (height == 9) {
              if (pixel(10) == 1) {
                return "K";
              } else /* NOLINT */ if (pixel(10) == 0) {
                return "Ч";
              } else /* NOLINT */ {}
            } else /* NOLINT */ {}
          } else /* NOLINT */ if (width == 7) {
            return "ж";
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      }
    }
  }
} else /* NOLINT */ {
  if (count < 27) {
    if (count < 23) {
      if (count == 19) {
        if (width == 5) {
          if (height == 9) {
            if (pixel(0) == 1) {
              if (pixel(1) == 1) {
                return "P";
              } else /* NOLINT */ if (pixel(1) == 0) {
                return "p";
              } else /* NOLINT */ {}
            } else /* NOLINT */ if (pixel(0) == 0) {
              return "б";
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (width == 6) {
          return "Э";
        } else /* NOLINT */ {}
      } else /* NOLINT */ if (count == 20) {
        if (width < 6) {
          if (width == 4) {
            return "Ё";
          } else /* NOLINT */ if (width == 5) {
            return "й";
          } else /* NOLINT */ {}
        } else /* NOLINT */ {
          if (width == 6) {
            return "ѳ";
          } else /* NOLINT */ if (width == 7) {
            return "O";
          } else /* NOLINT */ if (width == 9) {
            return "Ѵ";
          } else /* NOLINT */ {}
        }
      } else /* NOLINT */ if (count == 21) {
        if (width == 5) {
          return "Б";
        } else /* NOLINT */ if (width == 6) {
          return "д";
        } else /* NOLINT */ if (width == 7) {
          if (height == 9) {
            if (pixel(2) == 1) {
              return "ѣ";
            } else /* NOLINT */ if (pixel(2) == 0) {
              return "Л";
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (width == 8) {
          return "Ъ";
        } else /* NOLINT */ {}
      } else /* NOLINT */ if (count == 22) {
        if (width == 6) {
          return "Я";
        } else /* NOLINT */ if (width == 7) {
          if (height == 6) {
            return "м";
          } else /* NOLINT */ if (height == 9) {
            return "Ѣ";
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (width == 8) {
          if (height == 6) {
            return "ю";
          } else /* NOLINT */ if (height == 9) {
            return "A";
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      } else /* NOLINT */ {}
    } else /* NOLINT */ {
      if (count == 23) {
        if (width == 7) {
          if (height == 9) {
            if (pixel(1) == 1) {
              return "П";
            } else /* NOLINT */ if (pixel(1) == 0) {
              return "H";
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      } else /* NOLINT */ if (count == 24) {
        if (width == 5) {
          return "В";
        } else /* NOLINT */ if (width == 9) {
          return "ш";
        } else /* NOLINT */ {}
      } else /* NOLINT */ if (count == 25) {
        if (width == 5) {
          return "$";
        } else /* NOLINT */ if (width == 7) {
          if (height == 9) {
            return "Ѳ";
          } else /* NOLINT */ if (height == 11) {
            return "Ц";
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      } else /* NOLINT */ if (count == 26) {
        if (width == 7) {
          return "#";
        } else /* NOLINT */ if (width == 10) {
          return "%";
        } else /* NOLINT */ if (width == 11) {
          return "ту";
        } else /* NOLINT */ {}
      } else /* NOLINT */ {}
    }
  } else /* NOLINT */ {
    if (count < 32) {
      if (count == 27) {
        if (width == 7) {
          if (height == 9) {
            if (pixel(0) == 1) {
              return "И";
            } else /* NOLINT */ if (pixel(0) == 0) {
              return "Ф";
            } else /* NOLINT */ {}
          } else /* NOLINT */ {}
        } else /* NOLINT */ if (width == 10) {
          return "щ";
        } else /* NOLINT */ if (width == 11) {
          return "Ж";
        } else /* NOLINT */ {}
      } else /* NOLINT */ if (count == 28) {
        return "ф";
      } else /* NOLINT */ if (count == 30) {
        if (width == 8) {
          return "Д";
        } else /* NOLINT */ if (width == 14) {
          return "ТУ";
        } else /* NOLINT */ {}
      } else /* NOLINT */ if (count == 31) {
        if (width == 7) {
          return "Й";
        } else /* NOLINT */ if (width == 10) {
          return "Ю";
        } else /* NOLINT */ {}
      } else /* NOLINT */ {}
    } else /* NOLINT */ {
      if (count < 36) {
        if (count == 32) {
          return "M";
        } else /* NOLINT */ if (count == 33) {
          if (width == 9) {
            return "Ш";
          } else /* NOLINT */ if (width == 13) {
            return "ѳѵ";
          } else /* NOLINT */ {}
        } else /* NOLINT */ {}
      } else /* NOLINT */ {
        if (count == 36) {
          return "Щ";
        } else /* NOLINT */ if (count == 39) {
          return "КЛ";
        } else /* NOLINT */ if (count == 40) {
          return "@";
        } else /* NOLINT */ {}
      }
    }
  }
}
