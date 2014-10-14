#include <iostream>

using std::cout;
using std::endl;

class Message
{
public:
  Message(int count) : count(count)
  {
    cout << "new " << count << endl;
  }
  ~Message()
  {
    cout << "~ " << count << endl;
  }
  int count;
};

bool change(Message& msg)
{
  cout << "chan | ";
  int count = msg.count + 1;
  msg = std::move(Message(msg.count + 1));
  cout << "chan | ";
  return true;
}

int main()
{
  cout << "main | ";
  Message msg = Message(0);

  bool changed = change(msg);
  if (changed) cout << "main | changed" << endl;

  cout << "main | final " << msg.count << endl;

  cout << "main | ";
  return 0;
}