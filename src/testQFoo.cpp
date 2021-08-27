#include "Fixture.h"
#include <QTest>
#include "gtest/gtest.h"

class Foo : public QObject {
    Q_OBJECT
private slots:
    void t1() { QVERIFY(true); }
};

TEST( qTest, testFoo ) 
{
    auto app = Fixture::instance()->appRunner();
    app->wait( [&]()
    {
        Foo foo;
        QTest::qExec( &foo );
    });
}

#include "testQFoo.moc"
