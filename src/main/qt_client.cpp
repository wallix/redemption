/*

*/

 #include "../src/front/front_Qt_cmpl.hpp"


int main(int argc, char** argv){
    
    //" -name QA\\administrateur -pwd S3cur3!1nux -ip 10.10.46.88 -p 3389";

    QApplication app(argc, argv);

    int verbose = 511;

    Front_Qt front(argv, argc, verbose);
    
    
    app.exec();
    
}


