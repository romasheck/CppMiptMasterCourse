#if 0
set -e
CXX=g++
CXXFLAGS="-std=c++23 -Wall -Wextra"
SCRIPT_NAME=$(basename "$0" .cpp)
$CXX $CXXFLAGS "$0" -o "${SCRIPT_NAME}.out"
./"${SCRIPT_NAME}.out"
rm "${SCRIPT_NAME}.out"
exit
#endif

#include <iostream>
#include <cassert>

#include <iostream>
#include <cassert>

class List
{
private:
    struct Node
    {
        int value;
        Node* next;
        
        Node(int val, Node* nxt = nullptr)
            : value(val), next(nxt)
        {
        }
    };
    
    Node* m_head = nullptr;
    Node* m_tail = nullptr;

public:
    
    bool empty() const
    {
        return m_head == nullptr;
    }
    
    void show() const
    {
        Node* current = m_head;
        while (current != nullptr)
        {
            std::cout << current->value << " -> ";
            current = current->next;
        }
        std::cout << std::endl;
    }
    
    void push_front(int value)
    {
        Node* new_node = new Node(value, m_head);
        m_head = new_node;
        
        if (m_tail == nullptr)
            m_tail = m_head;
    }
    
    void push_back(int value)
    {
        Node* new_node = new Node(value);
        
        if (empty())
        {
            m_head = new_node;
            m_tail = new_node; 
        }
        else
        {
            m_tail->next = new_node;
            m_tail = new_node;
        }
    }
    
    int pop_front()
    {
        assert(!empty());
        
        Node* temp = m_head;
        m_head = m_head->next;
        
        if (m_head == nullptr)
            m_tail = nullptr;
        
        int val = temp->value;
        delete temp;
        return val;
    }
    
    int pop_back()
    {
        assert(!empty());
        
        if (m_head == m_tail)
        {
            int val = m_head->value;
            delete m_head;
            m_head = nullptr;
            m_tail = nullptr;
            return val;
        }
        
        Node* current = m_head;
        while (current->next != m_tail)
            current = current->next;        
        
            
        int val = m_tail->value;
        delete m_tail;
        m_tail = current;
        m_tail->next = nullptr;
        return val;
    }

    int get() const
    {
        assert(!empty());
        
        Node* slow = m_head;
        Node* fast = m_head;
        
        while (fast != nullptr && fast->next != nullptr && fast->next->next != nullptr)
        {
            slow = slow->next;
            fast = fast->next->next;
        }
        
        return slow->value;
    }
    
    ~List()
    {
        while (!empty())
            pop_front();
    }
};

void test_empty()
{
    List list;
    assert(list.empty());
}

void test_push_front()
{
    List list;
    list.push_front(1);
    list.push_front(2);
    list.push_front(3);
    assert(!list.empty());
}

void test_push_back()
{
    List list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);
    assert(!list.empty());
}

void test_pop_front()
{
    List list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);
    
    assert(list.pop_front() == 1);
}

void test_pop_back()
{
    List list;
    list.push_back(1);
    list.push_back(2);
    list.push_back(3);
    
    assert(list.pop_back() == 3);
}

void test_get()
{
    List list;
    
    list.push_back(5);
    assert(list.get() == 5);
    
    list.push_back(10);
    assert(list.get() == 5);
    
    list.push_back(15);
    assert(list.get() == 10);
    
    list.push_back(20);
    list.push_back(25);
    assert(list.get() == 15);
    
}

void test_integration()
{
    List list;
    
    list.push_front(10);
    list.push_back(20);
    list.push_front(5);
    list.push_back(25);
    
    assert(list.get() == 10);
    list.pop_front();
    list.pop_back();    
    assert(list.get() == 10);
}

void test_destructor()
{
    List* list = new List;
    list->push_back(1);
    list->push_back(2);
    list->push_back(3);
    delete list;
}

int main()
{
    test_empty();
    test_push_front();
    test_push_back();
    test_pop_front();
    test_pop_back();
    test_get();
    test_integration();
    test_destructor();
    
    List* list = new List;
    list->push_back(1);
    list->push_back(2);
    list->push_back(3);
    list->show();
    delete list;

    return 0;
}