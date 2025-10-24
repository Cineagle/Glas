# When will C++26 be available

**Logger class**
- Make all Mixins friends by declaring them in the class:  
  `friend Mixins...`
- Make the member function private:  
  `void enqueue(std::unique_ptr<Entry>&& entry);`
- Make the member variable private:  
  `const std::string loggerName;`
