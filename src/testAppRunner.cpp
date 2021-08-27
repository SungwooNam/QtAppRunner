#include "gtest/gtest.h"
#include "Fixture.h"
#include <iostream>
#include <QtWidgets>
#include <QMainWindow>
#include <QTest>
#include <QLineEdit>

using namespace std;

TEST( testAppRunner, helloWorld ) 
{
    auto app = Fixture::instance()->appRunner();
    QMainWindow* w = nullptr;
    int alreadyThere = 0;

    app->wait( [&]()
    {
        alreadyThere = QApplication::topLevelWidgets().size();
        w = new QMainWindow();
        w->setWindowTitle("Hello World");
        w->show(); 
        ASSERT_EQ( alreadyThere+1, QApplication::topLevelWidgets().size());
    });

    std::this_thread::sleep_for(100ms);
    app->wait( [&w,alreadyThere]()
    {
        ASSERT_EQ( alreadyThere+1, QApplication::topLevelWidgets().size());
        w->close(); 
        delete w;
        ASSERT_EQ( alreadyThere, QApplication::topLevelWidgets().size());
    }); 
}

TEST( testAppRunner, helloWorld1 ) 
{
    auto app = Fixture::instance()->appRunner();
    {
        unique_ptr<QMainWindow> w;
        app->wait( [&]()
        {
            w.reset( new QMainWindow() );
            w->setWindowTitle("Hello World");
            w->show(); 
        });

        app->wait( [&w]() 
        {
            w->close();
            w = nullptr;
        }); 
    }
}


TEST( testAppRunner, lineEditKeyClicks ) 
{
    auto app = Fixture::instance()->appRunner();
    QLineEdit* lineEdit = nullptr;

    app->wait( [&]()
    {
        lineEdit = new QLineEdit();
        lineEdit->show();
        QTest::keyClicks(lineEdit, "hello world");
        ASSERT_STREQ( "hello world", lineEdit->text().toStdString().c_str());
    });
    
    ASSERT_STREQ( "hello world", lineEdit->text().toStdString().c_str());

    app->wait( [&lineEdit]()
    {
        ASSERT_STREQ( "hello world", lineEdit->text().toStdString().c_str());
        lineEdit->close(); 
        delete lineEdit;
    }); 
}

TEST( testAppRunner, manyMainWindows ) 
{
    auto app = Fixture::instance()->appRunner();
    list<unique_ptr<QMainWindow>> mainWindows;

    for( int i = 0; i < 10; ++i )
    {
        app->dispatch( [&mainWindows,i]()
        {
            auto w = new QMainWindow();
            w->setWindowTitle( std::to_string(i).c_str() );
            w->show(); 
            mainWindows.push_back( unique_ptr<QMainWindow>(w));
        });
    }

    this_thread::sleep_for(100ms);
    app->wait( [&mainWindows]()
    {
        for( auto& w : mainWindows ) { w->close(); }
        mainWindows.clear();
    }); 
}

TEST( testAppRunner, wait ) 
{
    auto app = Fixture::instance()->appRunner();

    auto begin = chrono::system_clock::now();

        app->wait( []()
        {
            this_thread::sleep_for(100ms);
        }); 

    auto elapsed = chrono::duration_cast<chrono::milliseconds>( 
        chrono::system_clock::now() - begin).count();
    EXPECT_GE( 100, elapsed );
}

TEST( testAppRunner, dispatchThreadSafety ) 
{
    auto app = Fixture::instance()->appRunner();
    list<unique_ptr<QMainWindow>> mainWindows;
    vector<thread> threads;

    int alreadyThere = 0;
    app->wait( [&alreadyThere](){
        alreadyThere = QApplication::topLevelWidgets().size();
    });

    for( int i = 0; i < 10; ++i )
    {
        threads.push_back( thread( [app,&mainWindows,i](){
            app->dispatch( [&mainWindows,i]()
            {
                auto w = make_unique<QMainWindow>();
                w->setWindowTitle( std::to_string(i).c_str() );
                w->show(); 
                mainWindows.push_back(std::move(w));
            });
        }));
    }

    for( auto& t : threads ) { t.join(); }

    app->wait( [alreadyThere](){
        EXPECT_EQ( 10, QApplication::topLevelWidgets().size() - alreadyThere);
    });

    this_thread::sleep_for(100ms);
    app->wait( [&mainWindows,alreadyThere]()
    {
        for( auto& w : mainWindows ) { w->close(); }
        mainWindows.clear();
        EXPECT_EQ( 0, QApplication::topLevelWidgets().size() - alreadyThere);
    }); 
}