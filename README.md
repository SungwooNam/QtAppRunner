# QtAppRunner

Header only class to run QApplication on a separate thread. 

## Hello World 

```cpp
#include "AppRunner.hpp"

int main( int argc, char *argv[] )
{
    AppRunner app( argc, argv );

    QMainWindow* w = nullptr;
    app.wait( [&]()
    {
        w = new QMainWindow();
        w->setWindowTitle("Hello World");
        w->show(); 
    });

    std::this_thread::sleep_for(100ms);
    app.wait( [w]()
    {
        w->close(); 
        delete w;
    }); 

    return 0;    
}
```


