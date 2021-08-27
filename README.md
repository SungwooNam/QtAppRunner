# QtAppRunner

Header only class to run QApplication on a separate thread. 

The qTest provides QTEST_MAIN() but it can be specified only once. Hence gtest can't really test various scenarios of UI as there can be only a single point of execution. 

AppRunner is designed to overcome the restriction by running Qt at a dedicated thread. When AppRunner is created, it create a separate thread and execute QApplicaiton.processEvents() in the thread. And it gets user's lambda and executes the lambda function at the thread also. 


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

## gtest Qt stuff

### make a static AppRunner 

```cpp
#include "AppRunner.hpp"

class Fixture {
...
    static Fixture* instance() {        // Singleton Fixture holding AppRunner
        if(fixture_==nullptr){ fixture_ = new Fixture(); }
        return fixture_;
    }
    AppRunner* appRunner() { return &App_; }
...
private:
    AppRunner App_;
};
```

### gtest qtest QObject

```cpp
...
class TestGui: public QObject {
    Q_OBJECT
private slots:
    void testGui() {
        QLineEdit lineEdit;  
        QTest::keyClicks(&lineEdit, "hello world");
        QCOMPARE(lineEdit.text(), QString("hello world"));
    }    
};

TEST( qGuiTest, testGui ) {
    auto app = Fixture::instance()->appRunner();
    app->wait( [&]() {
        TestGui t;
        QTest::qExec( &t );
    });
}
```

### gtest QMainWindow

```cpp
TEST( testAppRunner, helloWorld ) 
{
    auto app = Fixture::instance()->appRunner();
    QMainWindow* w = nullptr;
    app->wait( [&]() {
        w = new QMainWindow();
        w->setWindowTitle("Hello World");
        w->show(); 
    });

    std::this_thread::sleep_for(100ms);
    app->wait( [&w,alreadyThere]() {
        w->close(); 
        delete w;
    }); 
}
```

### gtest many QMainWindow

```cpp
TEST( testAppRunner, many ) 
{
    auto app = Fixture::instance()->appRunner();
    list<unique_ptr<QMainWindow>> mainWindows;
    vector<thread> threads;

    for( int i = 0; i < 10; ++i ) {
        threads.push_back( thread( [app,&mainWindows,i](){
            app->dispatch( [&mainWindows,i]() {     // this lambda will be queued and executed at UI thread
                auto w = make_unique<QMainWindow>();
                w->setWindowTitle( std::to_string(i).c_str() );
                w->show(); 
                mainWindows.push_back(std::move(w));
            });  // dispatch() is not blocking and return immediately after queueing
        }));
    }

    for( auto& t : threads ) { t.join(); }          // make sure all lambdas are queued.

    app->wait( [&mainWindows]() {                   // this cleanup will be executed after all above lambda executed.
        for( auto& w : mainWindows ) { w->close(); }     
        mainWindows.clear();
    }); 
}
```