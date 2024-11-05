#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <queue>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <condition_variable>
#include <memory>
#include <functional>
#include <future>
#include <iostream>

// 线程安全队列
template<typename T>
class QueueGuard {
public:
    QueueGuard() = default;
    ~QueueGuard() = default;

    QueueGuard(const QueueGuard&) = delete;
    QueueGuard& operator=(const QueueGuard&) = delete;
    QueueGuard(QueueGuard&&) = delete;
    QueueGuard& operator=(QueueGuard&&) = delete;

    // 入队--写锁
    void push(const T& value) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        queue_.push(value);
        // cond.notify_one();
    }
    // 出队--写锁
    T pop() {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        T value = queue_.front();
        queue_.pop();
        // cond.notify_one();
        return value;
    }
    // 获取队列大小--读锁
    size_t size() {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return queue_.size();
    }
    bool empty() {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return queue_.empty();
    }
    

private:
    std::queue<T> queue_;
    std::shared_mutex mutex_;
};

/*  
    线程池类：
        1、单例模式
        2、任意线程任务（返回值、参数任意）
*/
class ThreadPool {

private:
    // 单例模式
    // ThreadPool():threads_(std::thread::hardware_concurrency()),is_shutdown_(false) {}
    ThreadPool():threads_(32), is_shutdown_(false) {}

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    // 线程池实例
    static std::shared_ptr<ThreadPool> instance_;
    static std::once_flag flag_;

    using TaskType = std::function<void()>;
    std::mutex lanch_;
    std::condition_variable cv_;
    // 内置线程工作类
    class worker {
    public:
        std::shared_ptr<ThreadPool> pool_;

        // worker的构造函数
        worker(std::shared_ptr<ThreadPool> pool): pool_(pool) {};
        // 重载()
        void operator() () {
            while(!pool_->is_shutdown_) {
                // 等待任务
                TaskType func;
                if(pool_->taskQueue_.empty()) {
                    // 如果任务队列为空，则等待任务
                    std::unique_lock<std::mutex> lock(pool_->lanch_);
                    pool_->cv_.wait(lock, [this]() {
                        return this->pool_->is_shutdown_ || !this->pool_->taskQueue_.empty();
                    });
                }
                // 取出任务
                if(!pool_->taskQueue_.empty()) {
                    func = pool_->taskQueue_.pop();
                }
                // 执行任务
                if(func != nullptr) {
                    func();
                }
            }
        }
    };

    private:
    // 线程池运行状态
    bool is_shutdown_ ;
    // 安全队列,存储类型为函数
    QueueGuard<std::function<void()>> taskQueue_;
    // 线程池
    std::vector<std::thread> threads_;
    // 线程池线程数
    int threadNum_;

public:
    ~ThreadPool() {
        shutdown();
    }
    static auto get_instance() ->std::shared_ptr<ThreadPool> {
        if(instance_ == nullptr) {
            std::call_once(flag_, [](){
                instance_ = std::shared_ptr<ThreadPool>(new ThreadPool());
            });
        }
        return instance_;
    }

    // 线程池初始化
    void init() {
        // 创建线程
        for(int i = 0; i < threads_.size(); i++) {
            // threads_.emplace_back(std::thread(worker(instance_)));
            threads_[i] = std::thread(worker(instance_));
        }
    }
    // 线程池添加任务
    template<typename F, typename... Args>
    auto submitTask(F&& f, Args&&...args) 
        -> std::future<decltype(f(args...))>;
        ;
    // 线程池关闭
    void shutdown() {
        // 确保所有任务执行完毕
        auto f = submitTask([]() {});
        f.get();
        {
            std::unique_lock<std::mutex> lock(lanch_);
            is_shutdown_ = true;
        }
        cv_.notify_all();
        for(auto& t : threads_) {
            if(t.joinable()) {
                t.join();
            }
        }
    }
    // 线程池运行状态
    bool is_shutdown() {
        return is_shutdown_;
    }
    // 线程池任务数
    int taskNum() {
        return taskQueue_.size();
    }
};

template<typename F, typename... Args>
auto ThreadPool::submitTask(F&& f, Args&& ...args) 
    -> std::future<decltype(f(args...))> {
    // using return_type = typename std::invoke_result_t<F, Args...>::type;
    using return_type = decltype(f(args...));
    
    // 将任务进行包装
    std::function<return_type()> taskWrapper1 = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

    auto taskWrapper2 = std::make_shared<std::packaged_task<return_type()>>(taskWrapper1);

    TaskType taskWrapper3 = [taskWrapper2]() {
        (*taskWrapper2)();
    };

    taskQueue_.push(taskWrapper3);
    cv_.notify_one();


    return taskWrapper2->get_future();
}

std::shared_ptr<ThreadPool> ThreadPool::instance_ = nullptr;
std::once_flag ThreadPool::flag_;


#endif