#ifndef QUEUE_H
#define QUEUE_H
#pragma once

#include <memory>
#include <mutex>
#include <queue>

/**
 * @brief �̰߳�ȫ�Ķ���
 * �����ṩһ�ֶ��̰߳�ȫ�Ķ���ʵ��
 * @tparam T ����Ԫ������
 */
template <typename T>
class SafeQueue
{
public:
    SafeQueue() = default;

    /**
     * @brief ���Ԫ�ص�����
     * ���һ��Ԫ�ص�������
     * @param value ����ӵ�Ԫ��
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
     * @brief �����Ƿ�Ϊ��
     * �������Ƿ�Ϊ��
     * @return true Ϊ��
     * @return false ��Ϊ��
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
     * @brief �ȴ�Ԫ�ز�����
     * ����������Ԫ��ʱ���Ӷ�����ȡ�ز�����Ԫ�أ�û��Ԫ��ʱ�ȴ���Ԫ�س���
     * @param outValue ������Ԫ��
     */
    void WaitAndPop(T& outValue)
    {
        std::unique_lock<std::mutex> lock{ m_Mutex };
        m_DataCondition.wait(lock, [this]() { return !m_Queue.empty(); });
        outValue = std::move(*m_Queue.front());
        m_Queue.pop();
    }

    /**
     * @brief �ȴ�Ԫ�ز�����
     * ����������Ԫ��ʱ���Ӷ�����ȡ�ز�����Ԫ�أ�û��Ԫ��ʱ�ȴ���Ԫ�س���
     * @param outValue ������Ԫ��
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
     * @brief �ȴ�Ԫ�ز�����
     * ����������Ԫ��ʱ���Ӷ�����ȡ�ز�����Ԫ�أ�û��Ԫ��ʱ�ȴ���Ԫ�س���
     * @return std::shared_ptr<T> ����Ԫ�ص�ָ��
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
     * @brief �ȴ�Ԫ�ز�����
     * ����������Ԫ��ʱ���Ӷ�����ȡ�ز�����Ԫ�أ�û��Ԫ��ʱ�ȴ���Ԫ�س���
     * @return std::shared_ptr<T> ����Ԫ�ص�ָ��
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
     * @brief ���ԴӶ����е���Ԫ��
     * ���ԴӶ����е���Ԫ�ش洢�� outValue ��
     * @param outValue ������Ԫ��
     * @return true ������ outValue ���޸�
     * @return false ����Ϊ�գ� outValue �����޸�
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
     * @brief ���ԴӶ����е���Ԫ��
     * ���ԴӶ����е���Ԫ�ز�������ָ��
     * @return std::shared_ptr<T> ����Ԫ�ص�ָ��
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
