#include "Fixture.h"
#include <QtWidgets>
#include <QtTest/QtTest>
#include "gtest/gtest.h"

class TestGui: public QObject
{
    Q_OBJECT

private slots:
    void testGui()
    {
        QLineEdit lineEdit;  
        QTest::keyClicks(&lineEdit, "hello world");
        QCOMPARE(lineEdit.text(), QString("hello world"));
    }    
};

TEST( qGuiTest, testGui ) 
{
    auto app = Fixture::instance()->appRunner();
    app->wait( [&]()
    {
        TestGui t;
        QTest::qExec( &t );
    });
}

#include "testQGui.moc"
