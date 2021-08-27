#ifndef __APP_RUNNER_HPP__
#define __APP_RUNNER_HPP__

#include <QApplication>
#include <thread>
#include <condition_variable>
#include <chrono>

class AppRunner 
{
public:
    AppRunner(int& argc, char **argv)
    {
        UIThread = std::thread{ [&]{
            App = new QApplication( argc, argv );
            App->setQuitOnLastWindowClosed(false);
            runInLoop();
        }};

        wait([](){});
    } 

    AppRunner()
    {
        UIThread = std::thread{ [&]{
            int argc = 0; 
            char **argv = nullptr;
            App = new QApplication( argc, argv );
            App->setQuitOnLastWindowClosed(false);
            runInLoop();
        }};

        wait([](){});
    }

    ~AppRunner()
    {
        postQuit();
        UIThread.join();
    }

    using UIActionFn = std::function<void()>;
    struct UIAction
    {
        UIActionFn Fn;
        bool Done;
    }; 

    void dispatch( UIActionFn&& act )
    {
        std::lock_guard<std::mutex> lock(MutexUIActions);
        UIActions.push_back( std::shared_ptr<UIAction>( new UIAction{ act, false }));
    }

    template<typename _Rep, typename _Period>
    bool wait_for( const std::chrono::duration<_Rep, _Period>& timeout, UIActionFn&& act )
    {
        std::shared_ptr<UIAction> pact( new UIAction{ act, false });
        
        {
            std::lock_guard<std::mutex> lock(MutexUIActions);
            UIActions.push_back( pact );
        }

        {
            std::unique_lock<std::mutex> lock(MutexActionDone);
            return IsDone.wait_for(lock, timeout, [pact]{return pact->Done;});
        }
    }

    void wait( UIActionFn&& act )
    {
        wait_for( std::chrono::seconds(std::numeric_limits<int>::max()), std::move(act) );
    } 
 

private:
    void runInLoop()
    {
        while( !isQuit() )
        {
            App->processEvents();

            for( auto& act : popActions() )
            {
                act->Fn();

                {
                    std::lock_guard<std::mutex> lock(MutexActionDone);
                    act->Done  = true;
                    IsDone.notify_all(); 
                }
            }
        }
    }

    std::vector<std::shared_ptr<UIAction>> popActions()
    {
        std::vector<std::shared_ptr<UIAction>> actions;
        {
            std::lock_guard<std::mutex> lock(MutexUIActions);
            std::copy( UIActions.begin(), UIActions.end(), back_inserter(actions));
            UIActions.clear();
        }
        return std::move(actions);
    }

    void postQuit()
    {
        App->closeAllWindows();

        {
            std::lock_guard<std::mutex> lock(MutexUIActions);
            Quit = true;
        }
        IsQuit.notify_all(); 
    }

    bool isQuit()
    {
        std::unique_lock<std::mutex> lock(MutexUIActions);
        return IsQuit.wait_for(lock, std::chrono::seconds(0), [this]{return Quit;});
    }

    QApplication* App;
    std::mutex MutexUIActions;
    std::condition_variable IsQuit;
    bool Quit = false;
    std::list< std::shared_ptr<UIAction> > UIActions;
    std::thread UIThread;
    std::mutex MutexActionDone;
    std::condition_variable IsDone;
};


#endif