all: common_classifier.hxx latin_classifier.hxx cyrillic_classifier.hxx

clean:
	$(RM) -r fonts classifiers

.PHONY: all clean

common_classifier.hxx: \
classifiers/segoeUI_8_bold.hxx \
classifiers/trebuchetMS_10_bold.hxx \
classifiers/segoeUI_11.hxx
	./cat_classifiers "$@" $^

latin_classifier.hxx: \
classifiers/tahoma_8_bold.hxx
	./cat_classifiers "$@" $^

cyrillic_classifier.hxx: \
classifiers/russian.hxx \
classifiers/cyrillic_tahoma_8_bold.hxx
	./cat_classifiers "$@" $^

classifiers/%.hxx: learning learn.ok/%/*.pbm learn.ok/%/*.txt
	mkdir -p fonts classifiers
	./learning.sh fonts/"$*".txt learn.ok/"$*"
	python ./gen_classifier.py fonts/"$*".txt "$@"

learning: ../../capture/ocr/learning.cc
	$(CXX) $^ -I../../capture/ocr/ -O2 -std=c++11 -o $@
