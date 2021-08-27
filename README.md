# QtAppRunner

Header only class to run QApplication on a separate thread. 

## Hello World 

```cpp
#include "AppRunner.hpp"

int main( int argc, char *argv[] )
{
    AppRunner app( argc, argv );    // create a thread and create QApplication in the thread 

    QMainWindow* w = nullptr;
    app.wait( [&]() {               // this lambda will be executed at the thread
        w = new QMainWindow();
        w->setWindowTitle("Hello World");
        w->show(); 
    });                             // return when above executed

    std::this_thread::sleep_for(100ms);
    app.wait( [w]() {               // then close the window at the thread 
        w->close(); 
        delete w;
    }); 

    return 0;    
}
```


