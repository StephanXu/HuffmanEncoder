#ifndef QUEUE_H
#define QUEUE_H
#pragma once

#include <memory>
#include <mutex>
#include <queue>

/**
 * @brief 线程安全的队列
 * 该类提供一种多线程安全的队列实现
 * @tparam T 队列元素类型
 */
template <typename T>
class SafeQueue
{
public:
    SafeQueue() = default;

    /**
     * @brief 添加元素到队列
     * 添加一项元素到队列中
     * @param value 待添加的元素
     */
    void Push(T value)
    {
        std::shared_ptr<T> data{ std::make_shared<T>(std::move(value)) };
        {
            std::lock_guard<std::mutex> lock{ m_Mutex };
            m_Queue.push(data);
            m_DataCondition.notify_one();
        }
    }

    /**
     * @brief 队列是否为空
     * 检查队列是否为空
     * @return true 为空
     * @return false 不为空
     */
    bool Empty() const
    {
        std::lock_guard<std::mutex> lock{ m_Mutex };
        return m_Queue.empty();
    }

    size_t Size() const
    {
        std::lock_guard<std::mutex> lock{ m_Mutex };
        return m_Queue.size();
    }

    /**
     * @brief 等待元素并返回
     * 当队列中有元素时，从队列中取回并弹出元素，没有元素时等待新元素出现
     * @param outValue 弹出的元素
     */
    void WaitAndPop(T& outValue)
    {
        std::unique_lock<std::mutex> lock{ m_Mutex };
        m_DataCondition.wait(lock, [this]() { return !m_Queue.empty(); });
        outValue = std::move(*m_Queue.front());
        m_Queue.pop();
    }

    /**
     * @brief 等待元素并返回
     * 当队列中有元素时，从队列中取回并弹出元素，没有元素时等待新元素出现
     * @param outValue 弹出的元素
     */
    void WaitAndPop(T& outValue, const bool& cancellatinFlag)
    {
        std::unique_lock<std::mutex> lock{ m_Mutex };
        m_DataCondition.wait(lock, [this, &cancellatinFlag]() { return !m_Queue.empty() || cancellatinFlag; });
        if (cancellatinFlag)
        {
            return;
        }
        outValue = std::move(*m_Queue.front());
        m_Queue.pop();
    }

    /**
     * @brief 等待元素并返回
     * 当队列中有元素时，从队列中取回并弹出元素，没有元素时等待新元素出现
     * @return std::shared_ptr<T> 弹出元素的指针
     */
    std::shared_ptr<T> WaitAndPop(const bool& cancellatinFlag)
    {
        std::unique_lock<std::mutex> lock{ m_Mutex };
        m_DataCondition.wait(lock, [this, &cancellatinFlag]() { return !m_Queue.empty() || cancellatinFlag; });
        if (cancellatinFlag)
        {
            return {};
        }
        std::shared_ptr<T> result{ m_Queue.front() };
        m_Queue.pop();
        return result;
    }

    /**
     * @brief 等待元素并返回
     * 当队列中有元素时，从队列中取回并弹出元素，没有元素时等待新元素出现
     * @return std::shared_ptr<T> 弹出元素的指针
     */
    std::shared_ptr<T> WaitAndPop()
    {
        std::unique_lock<std::mutex> lock{ m_Mutex };
        m_DataCondition.wait(lock, [this]() { return !m_Queue.empty(); });
        std::shared_ptr<T> result{ m_Queue.front() };
        m_Queue.pop();
        return result;
    }

    /**
     * @brief 尝试从队列中弹出元素
     * 尝试从队列中弹出元素存储到 outValue 中
     * @param outValue 弹出的元素
     * @return true 弹出， outValue 被修改
     * @return false 队列为空， outValue 不被修改
     */
    bool TryPop(T& outValue)
    {
        std::lock_guard<std::mutex> lock{ m_Mutex };
        if (m_Queue.empty())
        {
            return false;
        }
        outValue = std::move(*m_Queue.front());
        m_Queue.pop();
        return true;
    }

    /**
     * @brief 尝试从队列中弹出元素
     * 尝试从队列中弹出元素并返回其指针
     * @return std::shared_ptr<T> 弹出元素的指针
     */
    std::shared_ptr<T> TryPop()
    {
        std::lock_guard<std::mutex> lock{ m_Mutex };
        if (m_Queue.empty())
        {
            return std::shared_ptr<T>();
        }
        std::shared_ptr<T> result{ m_Queue.front() };
        m_Queue.pop();
        return result;
    }

private:
    std::queue<std::shared_ptr<T>> m_Queue;
    mutable std::mutex m_Mutex;
    std::condition_variable m_DataCondition;
};

#endif // QUEUE_H
