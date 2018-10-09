D=include/ocr1

all: $(D)/common_classifier.hxx $(D)/latin_classifier.hxx $(D)/cyrillic_classifier.hxx

clean:
	$(RM) -r $(D)/fonts $(D)/classifiers

.PHONY: all clean

$(D)/common_classifier.hxx: \
$(D)/classifiers/segoeUI_8_bold.hxx \
$(D)/classifiers/trebuchetMS_10_bold.hxx \
$(D)/classifiers/segoeUI_11.hxx
	./cat_classifiers "$@" $^

$(D)/latin_classifier.hxx: \
$(D)/classifiers/tahoma_8_bold.hxx
	./cat_classifiers "$@" $^

$(D)/cyrillic_classifier.hxx: \
$(D)/classifiers/russian.hxx \
$(D)/classifiers/cyrillic_tahoma_8_bold.hxx
	./cat_classifiers "$@" $^

$(D)/classifiers/%.hxx: learning learn.ok/%/*.pbm learn.ok/%/*.txt
	mkdir -p $(D)/fonts $(D)/classifiers
	./learning.sh $(D)/fonts/"$*".txt learn.ok/"$*"
	python ./gen_classifier.py $(D)/fonts/"$*".txt "$@"

learning: learning.cc
	$(CXX) $^ -I../../src/capture/ocr/ -I../../modules/ppocr/includes/mln -O2 -std=c++11 -o $@
