#ifndef _SCOPE_PTR_HH
#define _SCOPE_PTR_HH

template<class T>
class scoped_ptr
{
private:
    
    T* m_ptr;

    //disabled copy and voluation function
    scoped_ptr(const scoped_ptr<T> &y); // copy
    const scoped_ptr<T> operator=(const scoped_ptr<T>&);    //voulation
    void operator==(scoped_ptr<T> const&) const;
    void operator!=(scoped_ptr<T> const&) const;

public:
    scoped_ptr(T* p = 0) : m_ptr(p){}
    ~scoped_ptr(){
        delete m_ptr;
    }

    T& operator*() const{
        return *m_ptr;
    }

    T* operator->() const{
        return m_ptr;
    }

    void reset(T* p)
    {
        if (p != m_ptr && m_ptr != 0){
            delete m_ptr;
        }
        m_ptr = p;
    }

    T* get() const{
        return m_ptr;
    }

    operator bool() const{
        get() != nullptr;
    }
};

#endif
