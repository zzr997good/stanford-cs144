# CS144实验记录

# CheckPoint 0

## 1. 配置GNU/Linux环境

这门课的实验要求使用虚拟机来模拟GNU/Linux或者直接安装Ubuntu系统。很不巧，我使用的是M1芯片的Macbook air，不管是安装虚拟机或者装双系统或者使用docker来获得Ubuntu container都会对性能造成很大的影响。因此我使用Github Codespace，正好blank模版提供的也是Ubuntu 20.04.6的环境。

```bash
@zzr997good ➜ /workspaces/codespaces-blank $ cat /etc/os-release
NAME="Ubuntu"
VERSION="20.04.6 LTS (Focal Fossa)"
ID=ubuntu
ID_LIKE=debian
PRETTY_NAME="Ubuntu 20.04.6 LTS"
VERSION_ID="20.04"
HOME_URL="https://www.ubuntu.com/"
SUPPORT_URL="https://help.ubuntu.com/"
BUG_REPORT_URL="https://bugs.launchpad.net/ubuntu/"
PRIVACY_POLICY_URL="https://www.ubuntu.com/legal/terms-and-policies/privacy-policy"
VERSION_CODENAME=focal
```

接下来按照要求的安装对应的package

```bash
sudo apt update && sudo apt install git cmake gdb build-essential clang \
clang-tidy clang-format gcc-doc pkg-config glibc-doc tcpdump tshark
```

到此，环境配置完成。

## 2. 玩一玩networking

### 2.1 Fetch一个网页

要求是让你尝试直接用浏览器去访问一个网页和用telnet去发送HTTP REQUEST去访问一个网页，观察两者有什么不同。

直接用浏览器去访问： [http://cs144.keithw.org/hello](http://cs144.keithw.org/hello)，可以看出所有的发送请求的过程已经被浏览器包装起来了。作为一个client，唯一需要知道的就是网页的地址。

![Screenshot 2023-11-26 at 11.43.00 PM.png](CS144%E5%AE%9E%E9%AA%8C%E8%AE%B0%E5%BD%95%20c54a23d522b645d18d8868dafcdc442e/Screenshot_2023-11-26_at_11.43.00_PM.png)

使用telnet去访问：没有telnet工具的可以先安装一下telnet。然后需要建立和服务器的连接，然后发送HTTP Get Request，request中包含需要访问的URL路径和Host名称。然后关闭连接等待回复。

```bash
sudo apt install telnet
```

```bash
@zzr997good ➜ /workspaces/codespaces-blank $ telnet cs144.keithw.org http
Trying 104.196.238.229...
Connected to cs144.keithw.org.
Escape character is '^]'.
GET /hello HTTP/1.1
Host: cs144.keithw.org
Connection: close

#Response
HTTP/1.1 200 OK
Date: Mon, 27 Nov 2023 05:24:02 GMT
Server: Apache
Last-Modified: Thu, 13 Dec 2018 15:45:29 GMT
ETag: "e-57ce93446cb64"
Accept-Ranges: bytes
Content-Length: 14
Connection: close
Content-Type: text/plain

Hello, CS144!
Connection closed by foreign host.
```

### 2.2 调用stanford的smtp服务来给自己发送邮箱

我做不了，我不是Stanford的学生，我没有sunetid，我彻底失败。

### 2.3 双工通信

用netcat和telnet进行一个双向通信，就类似是两个人在一个聊天室聊天。你发送的信息会被echo在当前聊天室。

```bash
#terminal A
@zzr997good ➜ /workspaces/codespaces-blank $ netcat -v -l -p 9090
Listening on 0.0.0.0 9090
Connection received on localhost 39302
Hello there, here is A
Hello there, here is B
```

```bash
#terminal B
@zzr997good ➜ /workspaces/codespaces-blank $ telnet localhost 9090
Trying ::1...
Trying 127.0.0.1...
Connected to localhost.
Escape character is '^]'.
Hello there, here is A
Hello there, here is B
```

## 3. 用OS stream socket写一个网络程序去fetch一个网页

### 3.1 拉源代码

```bash
git clone https://github.com/cs144/minnow
```

想要把修改完的代码上传到自己仓库的话记得在codespace里面添加新的ssh：[https://docs.github.com/en/authentication/connecting-to-github-with-ssh/about-ssh](https://docs.github.com/en/authentication/connecting-to-github-with-ssh/about-ssh)

看样子整个项目都是用CMake进行build的，但是codespace本身自带的CMake的版本比较低，而项目要求3.24.2版本的CMake，因此我们需要重新安装一个CMake

```bash
#卸载旧版本CMake
sudo apt-get remove cmake
#去官网下载一个新版本的CMake的Linux x86-64的binary
#解压
tar -zxvf cmake-3.26.5-linux-x86_64.tar.gz
#安装
sudo mv cmake-3.26.5-linux-x86_64 /opt/cmake-3.26.5
#添加环境变量
vim ~/.bashrc
#export PATH=/opt/cmake-3.26.5/bin:$PATH
cmake --version
```

此时CMake已经是3.26.5版本的了，够用了

开始先编译build一遍源代码

```bash
cd minnow/
cmake -S . -B build
cmake --build build
#报错
```

我排查了一下问题，是我的gcc和g++的版本太低了。

按照教程[https://www.ovenproof-linux.com/2016/09/upgrade-gcc-and-g-in-ubuntu.html](https://www.ovenproof-linux.com/2016/09/upgrade-gcc-and-g-in-ubuntu.html) 把gcc和g++升级到了13。重新编译build一下，出现了以下错误。

![Screenshot 2023-11-27 at 4.58.59 PM.png](CS144%E5%AE%9E%E9%AA%8C%E8%AE%B0%E5%BD%95%20c54a23d522b645d18d8868dafcdc442e/Screenshot_2023-11-27_at_4.58.59_PM.png)

全是关于uint64_t的，去TMD的，直接暴力往byte_stream.hh里面添加头文件<cstdint>，问题解决。

![Screenshot 2023-11-27 at 5.00.07 PM.png](CS144%E5%AE%9E%E9%AA%8C%E8%AE%B0%E5%BD%95%20c54a23d522b645d18d8868dafcdc442e/Screenshot_2023-11-27_at_5.00.07_PM.png)

### 3.2 看看modern C++圣经

### 3.3 读一下file_descriptor提供的API

### 3.4 实现get_URL

其实就是用已经封装好的TCPSocket类来和服务器进行连接，然后发送按照格式发送request

```bash
void get_URL( const string& host, const string& path )
{
  // cerr << "Function called: get_URL(" << host << ", " << path << ")\n";
  // cerr << "Warning: get_URL() has not been implemented yet.\n";
  TCPSocket sock;
  Address server(host,"http");
  // build connection with the server
  sock.connect(server);
  // send request to the server
  string requst="GET "+path+" HTTP/1.1\r\n"+"Host: "+host+"\r\n"+"Connection: close\r\n"+"\r\n";
  sock.write(requst);
  // receive response from the server
  string response;
  while(!sock.eof()){
    sock.read(response);
    cout<<response;
  }
  sock.close();
}
```

测试一下

```bash
rm -rf build/
cmake -S . -B build 
cmake --build build
cmake --build build --target check_webget
```

测试结果都通过

![Screenshot 2023-11-27 at 10.18.48 PM.png](CS144%E5%AE%9E%E9%AA%8C%E8%AE%B0%E5%BD%95%20c54a23d522b645d18d8868dafcdc442e/Screenshot_2023-11-27_at_10.18.48_PM.png)

## 4. 设计一个bytestream类

要求设计一个单线程的在本地的bytestream类，有两个继承类Writer和Reader。其实就是一个生产者消费者模型，同时不用考虑同步问题。bytestream有一个capacity，表示该byetestream的buffer中最多能够在内存中占用多少空间。Writer写的时候，最多能够把buffer占满，剩下的数据就自动舍弃。(这一点很迷)。Reader读的时候可以从中取出一定长度的字节并放到自己的buffer里去。由于是在本地的bytestream，因此**可以把writer想象成TCP的buffer，receiver想象成application获取数据buffer。**

bytestream的buffer我觉得主要有以下几种实现方法

1. 使用queue：这是最显而易见的，因为它头文件自动帮你加了<queue>，但是因为C++的queue本身是可扩容的，用capacity去限制size需要特别注意push的时候不能超过capacity。因此push的时候push的有效长度应该为min(capacity-que.size(),data.size())
2. 使用vector：使用vector去模拟一个queue也是很常见的做法。push的时候不断push_back，pop的时候就用一个指针去维护当前stream的头部位置。不过这种实现方法太占内存，vector会一直扩容。
3. 用vector去模拟一个循环队列：固定vector的size是capacity，然后用两个指针去模拟队列头尾。每次移动都进行取模操作。

我用循环队列去实现。结果实现下来发现peek()真的很难用循环队列去返回一个string_view。

```bash
string_view Reader::peek() const
{
  // Your code here.
  if(is_finished()){
    throw runtime_error("Try to peek from finished stream");
  }
  if(has_error()){
    throw runtime_error("Try to peek from errored stream");
  }
  if(bytes_buffered_==0){
    return "";
  }
  size_t head = bytes_popped_ % capacity_;
  string ret="";
  for(size_t i=0;i<bytes_buffered_;i++){
    ret+=buffer_[head];
    head=(head+1)%capacity_;
  }
  return ret;
}
```

因为最后返回的bytes可能是是循环队列的尾部和头部连接，没办法直接返回string_view。如果临时产生一个string，返回的过程中string ret被销毁，string_view直接没有意义。于是我灰溜溜的改用queue去做buffer_了，如果用string去作为buffer_也可以，但是pop的时候需要用substr去产生新的buffer_比较麻烦。

但是当我用queue<char>去实现的时候，我发现第八个测试死活都会出现stack overflow的错误，排查了很久才知道原因。因为queue底层是使用deque实现的，而deque是当内存超过512B的时候就会分配一块新的512B的block，然后在指针数组中添加一个新的指针指向新的block的首地址。**因此queue<char>的内存超过512次push后并不连续，在返回string_view的时候就会出现stack overflow的错误。因为string_view需要一块连续内存的视图。**

最后我用string作为buffer_实现，每次pop的时候生成一个新的string。

```bash
string_view Reader::peek() const
{
  // Your code here.
  if(is_finished()){
    return string_view();
  }
  if(has_error()){
    return string_view();
  }
  if(buffer_.empty()){
    return string_view();
  }
  return string_view(buffer_);
}
```

这个peek的API真的恶心到我了，为了追求效率一定要返回string_view。

## 5. 提交代码

提交之前有以下几点要确认：

1. 测试全都通过了
2. 代码格式已经规范化了

```bash
cmake --build build --target check0
cmake --build build --target format
```

我在运行format用clang format规范代码格式的时候发现最后一行PackConstructorInitializers: NextLine无法识别。肯定又是clang的版本太低导致的，于是我更新了clang的版本

```bash
#下载安装clang18
wget https://apt.llvm.org/llvm.sh
chmod +x llvm.sh
sudo ./llvm.sh 18
sudo apt install clang-format-18
#切换clang版本
sudo update-alternatives --install /usr/bin/clang-format clang-format /usr/bin/clang-format-18 100
```

然后再次规范化代码即可。

# CheckPoint 1

## 1. 拉源码

```bash
git fetch
git merge origin/check1-startercode
cmake -S . -B build
cmake --build build
```

## 2. 实现一个Reassembler

这次要求实现一个Reassembler，这样receiver可以使用这个Reassembler去将由IP传来的datagram进行重组和去重，TCP的reliable也由此实现。

### 2.1 实现原理

![Screenshot 2023-12-01 at 2.19.16 PM.png](CS144%E5%AE%9E%E9%AA%8C%E8%AE%B0%E5%BD%95%20c54a23d522b645d18d8868dafcdc442e/Screenshot_2023-12-01_at_2.19.16_PM.png)

其实就是一个滑动窗口，对于receiver而言，数据流动的方向为，IP→Reassembler→Writer ByteStream→Application。蓝色部分表示已经发送给application的数据，因此不需要进行任何的存储。绿色部分表示由Reassembler排好的紧接在已经发送好的数据后的连续的数据，这一部分数据可以随时发送给application，因此缓存在Writer Bytestream中，等待application读取。红色部分表示提前发来的在capacity(window) 之内的数据，由于前面还缺少一些数据让他们成为有序的连续数据，因此可以先缓存在Reassembler中，等待缺失的数据到了以后再联合成有序的连续数据发送给Writer。超出capacity(first unacceptable index)的部分直接抛弃掉。

这次提供的接口主要就两个

```cpp
void insert( uint64_t first_index, std::string data, bool is_last_substring, Writer& output );
uint64_t bytes_pending() const;
```

然后我的实现方法就是在Reassembler类里面维护一些私有数据成员还有两个私有函数成员

```cpp
private:
  struct segment
  {
    uint64_t first_index;
    uint64_t len;
    std::string data;
    bool operator<( const segment& other ) const
    {
      return first_index == other.first_index ? len < other.len : first_index < other.first_index;
    }
  };
  std::set<segment> unassembled_segments;
  uint64_t first_unassembled_index;
  uint64_t bytes_pended;
  bool got_last_substring;

  std::set<segment>::iterator try_merge( std::set<segment>::iterator cur_data );
  std::set<segment>::iterator merge_overlapped( std::set<segment>::iterator prev_seg,
                                                std::set<segment>::iterator cur_data );
```

1. segment数据结构表示在Reassembler buffer里面存储的离散的字节流。
2. unassembled_segments就相当于是Reassembler buffer
3. first_unassembled_index就是如上图所示的第一个不连续的index，其值应该等于Writer.bytes_pushed()
4. bytes_pended表示在Reassembler buffer存储的所有离散的bytes的总长度
5. got_last_substring表示Reassembler是否已经接收到了last substring
6. try_merge()和merge_overlapped()是在insert()的时候用来尝试将Reassembler buffer中可以合并的bytes合并成连续区间的
7. 由于insert的时候，提供了output，因此first_unacceptable_index可以通过first_unassembled_index+output.available_capacity()计算出来
8. first_unpopped_index其实就是Reader.bytes_popped()，不过这个是由application的读取速度决定的，当application读取的快，窗口就移动的快，否则窗口就移动的慢。很多字节都变成了unacceptable
9. 整个capacity其实就是output的capacity，只不过从first_unpopped_index到first_unassembled_index那部分被buffer在了Writer的buffer中，而available capacity不仅仅是Writer的剩余capacity，也是Reassembler buffer的整个capacity。
10. 新来的数据首先要进行切割，保证其开头index不会小于first_unassembled_index，结尾不会大于first_unacceptable_index，这样可以保证Reassembler buffer和Writer的buffer(如果成功push到writer之后)也不会溢出
11. 接着对切割后的数据进行尝试merge，这是一个递归的过程，如果该数据无法和前面也无法和后面的数据进行merge，那么说明该数据是离散数据。无法merge，直接存入set。否则要让其和前后的数据merge得到新的数据，然后将新的数据继续和前后的数据进行merge，直到无法merge。
12. 判断merge之后的数据start_index是否为first_unassembled_index，如果是的话，将其从set中弹出，直接push到output的buffer中。
13. 在merge和弹出的过程中不断对bytes_pended进行加减操作。

具体实现代码请看代码仓库。

### 2.2 FAQ

## 3. 如何在测试的过程中debug

在CheckPoint0里面我debug都是凭感觉debug，也没用gdb，也磕磕绊绊过了。这次test比较多，感觉实在不方便，就用了vscode自带的gdb插件进行debug。毕竟有UI，又可以方便地打断点，还可以动态监视所有的变量。好用！

由于我是在codespace里面开发的，所以主要就是在/workspaces/codespaces-blank就是我的current working directory，然后minnow就在cwd下面，因此只需要在cwd里面创建一个.vscode文件夹(作为working directory的子目录)，然后在.vscode下面添加launch.json和task.json就可以随便打开/workspaces/codespaces-blank/minnow/tests/下任何一个测试文件打断点进行debug了。具体可以参考[https://segmentfault.com/a/1190000039087458](https://segmentfault.com/a/1190000039087458)

## 4. Submit

还是老样子

1. 检查test过了没
    
    ![Screenshot 2023-12-02 at 3.28.48 PM.png](CS144%E5%AE%9E%E9%AA%8C%E8%AE%B0%E5%BD%95%20c54a23d522b645d18d8868dafcdc442e/Screenshot_2023-12-02_at_3.28.48_PM.png)
    
2. 格式化代码

```bash
cmake --build build --target format
```

1. commit并提交代码到仓库

# CheckPoint 2

## 0. 背景故事

一个TCPReceiver，主要负责以下两个工作：

1. 接收从TCPSender发来的消息，把data放进reassembler。
2. 发送ack包，包中包含ackno和windowsize。

## 1. 拉源代码

老样子

```bash
git fetch --all
git merge origin/check2-startercode
git merge upstream/check2-startercode
cmake -S . -B build
cmake --build build
```

## 2. 实现TCPReceiver

这次要实现的TCPReceiver就是一个很简单的Receiver，他不会发送任何数据包给TCPSender，只是根据收到的packet来发送ack包。

### 2.1 实现64bit index到32bit seqno的转化

由于CheckPoint1和CheckPoint0里的bytestream index都是64位的，但在tcp header里只有32位用来存放seqno，因此我们需要用32位循环的seqno来表示64位的bytestream index。源代码中已经将uint32_t封装成了Wrap32类，其中包含两个方法：

```cpp
static Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
uint64 t unwrap( Wrap32 zero point, uint64_t checkpoint ) const
```

其中静态wrap方法使用来将一个64位的Absolute Sequence Numbers(从0开始的seqno)转换成32位的seqno，同时需要提供一个zero_point，这是TCP三次握手时Sender发送的随机生成的SYN的seqno，也就是ISN。而方法unwrap就来将当前Wrap32对象转换成最靠近checkpoint的Absolute Sequence Numbers。也就是说Wrap32提供了squno和Absolute Sequence Numbers之间的转换。至于Absolute Sequence Numbers和bytestream index之间的转换非常简单。具体可以参考下图。

![Screenshot 2023-12-05 at 2.03.26 PM.png](CS144%E5%AE%9E%E9%AA%8C%E8%AE%B0%E5%BD%95%20c54a23d522b645d18d8868dafcdc442e/Screenshot_2023-12-05_at_2.03.26_PM.png)

也就是说bytestream index是不包括SYN和FIN的仅含data的0-index，也就是Reassembler和Writer中使用的index，而Absolute Sequence Numbers是包含SYN，FIN和data的0-index。至于seqno则是根据一个随机产生的ISN将Absolute Sequence Numbers转换成32位循环形式的包含在返回包中的index。

接下来实现这两个API。

```cpp
Wrap32 Wrap32::wrap( uint64_t n, Wrap32 zero_point )
{
  return zero_point + static_cast<uint32_t>( n );
}

uint64_t Wrap32::unwrap( Wrap32 zero_point, uint64_t checkpoint ) const
{
  // Step1: wrap the checkpoint and find the offset in uint32_t range
  uint32_t offset = raw_value_ - wrap( checkpoint, zero_point ).raw_value_;
  // Step2: get the potential result
  uint64_t result = checkpoint + offset;
  // Why this is a potential result?
  // 1. rwo_value_ could be smaller than wrapped checkpoint and in that case offset is wrap to a bigger dis
  // 2. offset could be bigger than 2^31 which cause offset-2^32 is a closer result
  if ( offset > ( 1u << 31 ) && result >= ( 1ull << 32 ) ) {
    result -= ( 1ull << 32 );
  }
  return result;
}
```

wrap的实现原理较为简单，直接将Absolute Sequence Numbers静态cast成32位，这和%(1<<32)的效果一样，得到在32位下的偏移量，然后将偏移量加上zero_point。

unwrap的实现比较tricky。因为不管是在64位数轴上还是32位数轴上，两个数字之间的偏移量是不变的。因此我们首先计算转换之后的checkpoint和当前raw_value_在32位数轴上的偏移量。接着将其加到64位checkpoint上，就得到了还原之后可能的结果。为什么说只是可能的结果呢？因为有以下几种情况：

1. 转换之后的checkpoint比当前raw_value_大，计算出来的偏移量因为也是uint32_t的，会从一个负数k变成2^32-k，这就导致还原之后得到了一个比checkpoint大并且较远的数字。
2. raw_value_比转换之后的checkpoint大，但是偏移量大于2^31，也就是整个区间的一半。那么得到的结果虽然是正常unwrap的结果，但是前面有一个离checkpoint更近的。
    
    ![Screenshot 2023-12-12 at 12.51.06 PM.png](CS144%E5%AE%9E%E9%AA%8C%E8%AE%B0%E5%BD%95%20c54a23d522b645d18d8868dafcdc442e/Screenshot_2023-12-12_at_12.51.06_PM.png)
    

因此当offset>(1<<31)并且得到的结果比(1<<32)大时，可以减去一个区间长度，得到一个离checkpoint更近的结果。

实现完之后可以单独进行测试，如果按照材料所说用以下这行命令进行测试肯定无法通过，因为他单独加了一些testcase。

```bash
cmake --build build --target check2
```

所以我搜了一下单独进行测试用例的办法：

```bash
#首先列出所有的测试用例
cmake --build build --target help
#然后用ctest --test-dir指定build目录
#ctest -R 指定运行的测试
ctest --test-dir build -R wrapping_integers_wrap
ctest --test-dir build -R wrapping_integers_unwrap
```

### 2.2 实现TCPReceiver类

![Screenshot 2023-12-05 at 2.20.33 PM.png](CS144%E5%AE%9E%E9%AA%8C%E8%AE%B0%E5%BD%95%20c54a23d522b645d18d8868dafcdc442e/Screenshot_2023-12-05_at_2.20.33_PM.png)

TCPSender发来的消息主要包含以下几个数据：

1. 是不是SYN信号
2. 是不是FIN信号
3. 包含的data payload
4. 发送的seqno

如果发送的包含SYN信号，那么seqno是由sender随机生成的ISN，也就是SYN信号的seqno。如果不包含SYN信号，并且payload不为空，那么seqno是payload第一位byte的seqno。如果仅包含FIN信号，那么seqno是FIN的seqno。也就是说SYN和FIN都会占用一位seqno。

![Screenshot 2023-12-05 at 2.33.46 PM.png](CS144%E5%AE%9E%E9%AA%8C%E8%AE%B0%E5%BD%95%20c54a23d522b645d18d8868dafcdc442e/Screenshot_2023-12-05_at_2.33.46_PM.png)

TCPReceiver收到Sender发来的消息后产生的信息只包含两个数据：

1. 回复的ackno
2. windowsize来进行flow control

![Screenshot 2023-12-05 at 2.35.02 PM.png](CS144%E5%AE%9E%E9%AA%8C%E8%AE%B0%E5%BD%95%20c54a23d522b645d18d8868dafcdc442e/Screenshot_2023-12-05_at_2.35.02_PM.png)

TCPReceiver要实现的两个API已经提及过了。

实现思路：

1. 首先TCPRecerver中要保存一个私有数据成员用来表示是否收到了SYN信号

```cpp
std::optional<Wrap32> zero_point { std::nullopt };
```

1. receive API的实现：
    1. 首先检查zero_point是否有值并且当前message是否包含SYN，如果都没有，那就直接返回，因为连接还没建立。
    2. 如果当前message包含SYN，说明这是SYN包(也可能包含data和FIN信号)，将zero_point赋值为message中包含的ISN(seqno)
    3. 计算message的bytestream index：方法是先将seqno逆转换成Absolute Sequence Numbers，然后根据Absolute Sequence Numbers和bytestream index的关系来计算bytestream index。那么逆转换的checkpoint是多少呢？材料要求用reassembler中的first unassembled index来作为checkpoint，也就是bytes_pushed()。但是bytes_pushed()返回的是bytestream index而不是Absolute Sequence Numbers，二者的含义是不同的，因此还需要加上SYN在Absolute Sequence Numbers中占的一位，因此checkpoint是1+bytes_pushed()。
    4. 计算bytestream index：如果该message中包含SYN，那么计算得到的Absolute Sequence Numbers必然为0，此时包含payload的bytestream index也必然是从0开始。如果不包含SYN，那么Absolute Sequence Numbers是包含了SYN的index，因此需要讲得到的Absolute Sequence Numbers-1才是最终的bytestream index。
    5. 得到bytestream index后就可以用以下的API来向reassembler发送数据。
    
    ```cpp
    reassembler.insert( first_index, message.payload, message.FIN, inbound_stream );
    ```
    
2. send API的实现
    1. ackno的填写：主要是把当前收到的所有数据量wrap成一个Wrap32类型。如果当前已经收到了SYN(zero_point有值)，那么SYN要占一位。如果当前已经收到了FIN(inbound_stream.is_closed())，那么FIN要占一位。另外所有的data_pushed()也需要占位，把以上三者考虑起来就得到了当前接收到的字节量，也就是期待的下一个byte的Absolute Sequence Numbers。那么将其根据zero_point包装成wrap32类型的数据并赋值就行。
    2. window size的填写：注意window size是16位的，而inbound_stream.available_capacity()是32位的，因此需要截断。如果inbound_stream.available_capacity()>0xFFFF，那么window size最大也就只能是0xFFFF。

## 3. 测试

老样子gdb慢慢调试，这次他添加了一些对reassembler的测试用例，需要因此我也改动了reassembler的一个小地方，也就是完全overlapping的数据如果已经在set中，那么就insert就不用继续考虑merge了。

![Screenshot 2023-12-05 at 3.19.22 PM.png](CS144%E5%AE%9E%E9%AA%8C%E8%AE%B0%E5%BD%95%20c54a23d522b645d18d8868dafcdc442e/Screenshot_2023-12-05_at_3.19.22_PM.png)

## 4. 提交代码

老样子，没什么好说的

## 5. 自己找点test case

交给测试做吧

# CheckPoint3

## 0. 背景故事

这次要求实现一个TCPSender类，主要负责：

1. 根据从receiver接收到的message不断移动窗口，并且调整窗口大小
2. 从App的outbound_stream不断读取数据放入要发送的窗口中，尽可能填满窗口，并且发送数据
3. 如果已经发送的数据超过了Timer还没有被ack，那么将其放入窗口等待重传

![Screenshot 2023-12-12 at 1.29.19 PM.png](CS144%E5%AE%9E%E9%AA%8C%E8%AE%B0%E5%BD%95%20c54a23d522b645d18d8868dafcdc442e/Screenshot_2023-12-12_at_1.29.19_PM.png)

## 1. 拉代码

```cpp
git fetch --all
git merge origin/check3-startercode 
cmake -S . -B build
cmake --build build
```

## 2. 实现TCP Sender

实现一个如上所说的TCP Sender

### 2.1 怎么知道发出去的消息需要重传

需要实现一个Timer类。已经发送的segment要么被Receiver发送的message ACK，要么由于Timer超过了设置的RTO需要被重传。其中在测试的时候由人为进行调用tick函数来让Timer增加，一旦Timer增加超过RTO，需要在tick函数中来实现重传。教材中描述的关于重传规则有点混乱，我会在下一个部分解释每个函数实现的时候进行详细描述。

### 2.2 实现TCP Sender

主要有以下几个部分

1. 需要实现一个Timer，维护当前的tick和Timer运行状态。并且提供一些api包括
    1. 对tick进行自增操作并且检测是否超过了RTO
    2. 将Timer停止
    3. 重启Timer并将tick重置为0
    4. 得到Timer当前的tick
    5. 得到Timer当前的运行状态
    
    ```cpp
    class Timer
    {
    private:
      uint64_t ticket_ = 0;
      bool is_running_ = false;
    
    public:
      // Timer():ticket_(0),is_running_(false){}
      void start()
      {
        ticket_ = 0;
        is_running_ = true;
      }
      void stop() { is_running_ = false; }
      bool is_running() const { return is_running_; }
      uint64_t get_ticket() const { return ticket_; }
      bool is_expired( uint64_t ms_since_last_tick, uint64_t RTO_ms )
      {
        ticket_ += ms_since_last_tick;
        return is_running_ && ( ticket_ >= RTO_ms );
      }
    };
    ```
    
2. TCPSender中维护一些变量包括
    1. isn_：表示Sender用来建立connection时随机生成的isn
    2. initial_RTO_ms_：表示最初的RTO
    3. consecutive_retransmissions_：表示同一个包连续重传了多少次
    4. rcv_abs_ackno_：表示上次从receiver收到的ackno
    5. window_size_：表示上次的reverver收到的window_size
    6. abs_seqno_：表示需要下一个要发送的数据的abs_seqno
    7. bytes_in_flight_：表示有多少数据已经发送出去了但仍未被ack，应该等于abs_seqno_-rcv_abs_ackno_
    8. Timer：表示TCPSender用来计时的Timer
    9. sent_SYN_：表示是否发送了SYN
    10. sent_FIN_：表示是否发送了FIN
    11. _outstanding_messages：表示当前已经发送但是未被ack的消息队列
    12. _messages：表示已经被push进window的消息队列
    
    ```cpp
    Wrap32 isn_;
      uint64_t initial_RTO_ms_;
      uint64_t RTO_ms_ = initial_RTO_ms_;
      uint64_t consecutive_retransmissions_ { 0 };
      uint64_t rcv_abs_ackno_ { 0 };
      uint64_t window_size_ { 1 };
      uint64_t abs_seqno_ { 0 };
      uint64_t bytes_in_flight_ { 0 };
      Timer timer_ {};
      bool sent_SYN_ { false };
      bool sent_FIN_ { false };
      // store the sent but not acked message
      std::queue<TCPSenderMessage> _outstanding_messages {};
      // store the message to be sent
      std::queue<TCPSenderMessage> _messages {};
    ```
    
3. TCPSender API的实现
    1. sequence_numbers_in_flight：直接返回bytes_in_flight_
    2. consecutive_retransmissions：直接返回consecutive_retransmissions_
    3. maybe_send：
        1. 如果_messages队列为空，直接返回空。
        2. 如果consecutive_retransmissions_不为0，但是Timer正在运行，并且tick不为0，直接返回空。这是因为consecutive_retransmissions_不为0表示需要重传信息，但是重传信息需要初始化Timer，应该在tick为0时进行重传。
        3. 从_messages队列中pop出一条msg，然后返回msg。
    4. push:
        1. 如果sent_FIN_为真，直接返回。因为数据流已经被关闭了，没有数据可以继续被push到消息队列中等待发送。
        2. 根据rcv_abs_ackno_，abs_seqno_和window_size_，计算当前的available_space。注意如果window_size_为0的时候，需要将其当作1。然后不断从outbound_stream中读取数据生成message。首先根据isn和abs_seqno_来填写message的seqno。接着，如果未发送SYN要设置message的SYN为真。接着读取payload，尽量将available_space用满。如果available_space>0并且outbound_stream已经关闭了，还需要设置message中的FIN。其中SYN和FIN都需要占用available_space。接着将产生的message放入_outstanding_messages队列(体现默认为已经发送但是未被ack)和_messages队列。如果timer还没运行，需要让timer运行起来。然后产生下一条message。直到available_space为空。注意当产生的message的长度为0的时候(不包括SYN，payload和FIN)，说明APP还没有足够的数据提供给outbound_stream，这个时候不需要将这个空包放入消息队列，而是直接返回。在产生消息的过程中还需要不断维护bytes_in_flight_和abs_seqno_。
    5. send_empty_message:发送一个只含seqno的空包，主要用来让receiver回复ack信息。
    6. receive：
        1. 首先根据接收到的ackno来更新rcv_abs_ackno_。如果更新后的rcv_abs_ackno_大于abs_seqno_，直接返回。因为说明receiver想要的消息的seqno比我将要发送的消息的seqno还大了。出现了错误。
        2. 更新window_size
        3. 将_outstanding_messages队列中已经**完全被ack的消息**弹出队列，这些消息的seqno+len≤rcv_abs_ackno_。同时需要维护bytes_in_flight_。
        4. 如果有任何一条消息被完全ack(被弹出队列)，将RTO_ms_恢复为initial_RTO_ms_，并且将consecutive_retransmissions_重置为0。如果还有没有被ack的消息存在的话，重置timer。因为一次顺利的ack之后需要让timer重置。否则让timer停止，因为已经没有消息等待ack了，也就自然不需要计时了。
    7. tick：
        1. 如果timer加上ms_since_last_tick后没有超时，直接返回。
        2. 如果_outstanding_messages.empty()里没有消息，直接返回。因为没有东西要被重传。
        3. 把需要重传的消息(最早的消息)从_outstanding_messages放入_messages等待重传。注意，不要从_outstanding_messages中pop，因为可能重传失败，下次还要重传。
        4. 如果window_size_>0，需要把RTO_ms_翻倍。这可以让下次重传的tolerance更大。不至于同一个包被重传太多次。
        5. consecutive_retransmissions_自增并且重置Timer，此时的Timer用来计时重传的包。
        
        ```cpp
        TCPSender::TCPSender( uint64_t initial_RTO_ms, optional<Wrap32> fixed_isn )
          : isn_( fixed_isn.value_or( Wrap32 { random_device()() } ) ), initial_RTO_ms_( initial_RTO_ms )
        {}
        
        uint64_t TCPSender::sequence_numbers_in_flight() const
        {
          // Your code here.
          return bytes_in_flight_;
        }
        
        uint64_t TCPSender::consecutive_retransmissions() const
        {
          // Your code here.
          return consecutive_retransmissions_;
        }
        
        optional<TCPSenderMessage> TCPSender::maybe_send()
        {
          // If there is no message to send, return an empty optional
          if ( _messages.empty() ) {
            return {};
          }
          // If I am goint to retransmit a message, I should resend it in the reset timer, which means timer_.ticket=0
          if ( consecutive_retransmissions_ && timer_.is_running() && timer_.get_ticket() > 0 ) {
            return {};
          }
          auto msg = _messages.front();
          _messages.pop();
          return msg;
        }
        
        void TCPSender::push( Reader& outbound_stream )
        {
          // Your code here.
          // If TCP is closed, return
          if ( sent_FIN_ )
            return;
          auto available_space = rcv_abs_ackno_ + ( window_size_ ? window_size_ : 1 ) - abs_seqno_;
          while ( available_space > 0 && !sent_FIN_ ) {
            TCPSenderMessage msg;
            // If SYN has not been sent, send SYN
            msg.seqno = Wrap32::wrap( abs_seqno_, isn_ );
            if ( !sent_SYN_ ) {
              msg.SYN = true;
              sent_SYN_ = true;
              available_space--;
            }
            // Send the payload
            read( outbound_stream, std::min( available_space, TCPConfig::MAX_PAYLOAD_SIZE ), msg.payload );
            available_space -= msg.payload.size();
            // If the outbound_stream is finished, send FIN
            if ( available_space > 0 && outbound_stream.is_finished() ) {
              msg.FIN = true;
              sent_FIN_ = true;
              available_space--;
            }
            auto occupied = msg.sequence_length();
            // Could not read anything from the outbound_stream
            if ( occupied == 0 )
              return;
            // Push the message to _messages so that it could be sent later
            _messages.push( msg );
            // Push the message to _outstanding_messages because it is not acked yet
            _outstanding_messages.push( msg );
            bytes_in_flight_ += occupied;
            abs_seqno_ += occupied;
            if ( !timer_.is_running() ) {
              timer_.start();
            }
          }
        }
        
        TCPSenderMessage TCPSender::send_empty_message() const
        {
          // Your code here.
          TCPSenderMessage msg;
          msg.seqno = Wrap32::wrap( abs_seqno_, isn_ );
          return msg;
        }
        
        void TCPSender::receive( const TCPReceiverMessage& msg )
        {
          // Your code here.
          if ( msg.ackno ) {
            rcv_abs_ackno_ = msg.ackno->unwrap( isn_, abs_seqno_ );
          }
          // If the ackno is greater than abs_seqno_, the receiver needs future data, error happens
          if ( rcv_abs_ackno_ > abs_seqno_ ) {
            return;
          }
          window_size_ = msg.window_size;
          bool getAck = false;
          while ( !_outstanding_messages.empty() ) {
            // Try to remove the acked messages from _outstanding_messages
            auto& ealy_msg = _outstanding_messages.front();
            // This message is not completely acked,just partially acked
            if ( ealy_msg.seqno.unwrap( isn_, abs_seqno_ ) + ealy_msg.sequence_length() > rcv_abs_ackno_ ) {
              break;
            }
            bytes_in_flight_ -= ealy_msg.sequence_length();
            _outstanding_messages.pop();
            getAck = true;
          }
          if ( getAck ) {
            RTO_ms_ = initial_RTO_ms_;
            if ( !_outstanding_messages.empty() ) {
              timer_.start();
            } else {
              timer_.stop();
            }
            consecutive_retransmissions_ = 0;
          }
        }
        
        void TCPSender::tick( const size_t ms_since_last_tick )
        {
          if ( !timer_.is_expired( ms_since_last_tick, RTO_ms_ ) || _outstanding_messages.empty() )
            return;
          // Start retransmission
          _messages.push( _outstanding_messages.front() );
          if ( window_size_ > 0 ) {
            RTO_ms_ *= 2;
          }
          consecutive_retransmissions_++;
          // maybe_send();
          timer_.start();
        }
        ```
        
4. 其中有几点要特别说明一下。第一，所有的message当被push进消息队列的时候，就会很快地被maybe_send()发送出去，不存在说只发送窗口的一部分，因此_messages消息队列被push填入之后很快就会被清空。发送的过程就是清空_messages的过程。第二，对于一条消息，只可能有两种情况。要么在RTO之前被ack，那么这条消息会被从_outstanding_messages移除，因此receive的过程就是_outstanding_messages清空的过程。要么触发RTO，这条消息被重传，此时这条消息会被从_outstanding_messages放入_messages。并且重置Timer。接下来可以不断地call maybe_send()来将_messages里的消息(因为会因为窗口移动push进新的message)全都发送出去(consecutive_retransmissions_>0&&Timer.tick_==0)，由于发送速度非常快，在下一次tick_变化之前可以讲messages_消息队列清空。在Timer.tick_>0的时候，_messages就不再继续发消息，等待上一次重传的消息被Ack。如果继续RTO，那么同一个消息会继续被重传，直到被ack或者connection被断开。

### 2.3 FAQ

## 3. 测试

![Screenshot 2023-12-12 at 12.54.24 AM.png](CS144%E5%AE%9E%E9%AA%8C%E8%AE%B0%E5%BD%95%20c54a23d522b645d18d8868dafcdc442e/Screenshot_2023-12-12_at_12.54.24_AM.png)

## 4. 提交代码

老样子

## 5. 额外分

不做

# CheckPoint 4

## 0. 背景故事

这个实验要设计的其实是IP Layer的Interface。因为上层的TCP layer已经设计完成，接下来想要把TCP packet发送出去有三种办法。第一种是把TCP packet包装在UDP之中，然后通过UDPSocket发送出去。用户要做的就只是产生TCP packet，剩下的包装成UDP datagram，产生IP packet和Ethernet packet并发送都会由kernel来完成。第二种方法是最常见的TCP/IP，也就是说用户自己产生TCP packet，并且将TCP pakcet包装成IP packet，借助kernel的TUN接口来让kernel根据IP packet产生Ethernet packet(如果不知道destination就利用TUN实现的ARP接口)。第三个方法就是自己产生TCP packet，IP packet并且自己实现IP interface，调用IP interface来产生Ethernet packet，然后借助TAP接口来直接将产生的Ethernet进行发送。本次的目的就是来实现第三种方法中的IP interface。

## 1.  拉代码

```bash
git fetch --all
git merge origin/check4-startercode
git merge upstream/check4-startercode
cmake -S . -B build
cmake --build build
```

## 2. ARP

IP packet，Ethernet packet和ARP packet的数据结构在fetch的代码中已经实现好了，我们要做的就是去实现IP layer的network interface，主要负责以下几个功能。

1. 如果下一个hop的ethernet address我当前知道，那么直接将IP packet加上Ethernet header产生Ethernet packet发送出去
2. 如果下一个hop的ethernet address我不知道，那么我broadcast一个ARP request，并将要发送的IP packet缓存，等收到ARP reply，再将缓存的IP packet加上Ethernet header发送出去

我的实现方法如下：

NetworkInterface类中包含了以下的私有数据成员：

```cpp
 	EthernetAddress ethernet_address_;

  // IP (known as Internet-layer or network-layer) address of the interface
  Address ip_address_;

  // Timer
  uint64_t cur_ms { 0 };

  static constexpr uint64_t UPDATE_DICTIONARY_TIMER = 30000; // arp_list updated time
  static constexpr uint64_t RETRANSMIT_ARP_TIMER = 5000;     // ARP request retransmit time
  // Map of IP addresses to datagrams waiting for ARP replies
  std::unordered_map<IPAddr32, std::pair<EthernetAddress, uint64_t>> ARP_dictionary_ {};

  // Queue of Ethernet frames awaiting transmission
  std::queue<EthernetFrame> ready_to_send_ {};

  // Queue of ARP frames awaiting reply
  std::queue<ARP_query> wait_for_reply_ {};

  // IP data waiting for ARP reply to be sent
  std::unordered_map<IPAddr32, InternetDatagram> data_wait_for_ARP_reply_ {};
```

以下是每个私有数据成员的意义：

1. ethernet_address_：表示当前节点的ethernet_address
2. ip_address_：表示当前节点的ip address
3. cur_ms：表示计时器
4. UPDATE_DICTIONARY_TIMER：表示需要把dictionary里面存储的数据的有效时间段
5. RETRANSMIT_ARP_TIMER：表示ARP request发送出去后的有效期，在这个有效期内不需要再发送相同的请求
6. ARP_dictionary_：表示用来记录已知<ip_address_,<ethernet_address_，know_time>>的哈希表
7. ready_to_send_：表示可以发送的Ethernet packet
8. wait_for_reply_：表示发送出去并且等待回复的ARP request
9. data_wait_for_ARP_reply_：表示因为等待ARP request而被缓存起来的<ip_address_,InternetDatagram>数据

其中IPAddr32是我自己用来表示32位IP的数据结构，因为如果想用Address类型作为hashmap的key，就不得不为Address类型编写hash()函数，因此我直接用IPAddr32来表示32位IP更加方便。另外ARP_query也是我自己设计的数据结构，把request本体和发送时间还有查询的IP封装到了一个结构里面。以下是两种数据结构：

```cpp
using IPAddr32 = uint32_t;

struct ARP_query
{
  IPAddr32 ip;
  EthernetFrame frame;
  uint64_t time;
};
```

以上的私有数据主要用来实现NetworkInterface的以下API：

1. maybe_send()：看ready_to_send里面有没有ethernet packet
    1. 如果有的话，将最早的packet弹出并且返回该packet。
    2. 如果没有，返回空。
2. send_datagram()：看要发送的IP是否已经已经知道其ethernet address。
    1. 如果已经知道并且还没过期，那么调用函数send_Ethernet_frame()
    2. 如果还不知道或者已经过期了，那么调用函数send_ARP_request()
3. recv_frame()：
    1. 如果ethernet packet的类型是ipv4并且ethernet header的地址是我并且能够将payload解析成IP packet，那么直接返回解析后的IP packet
    2. 如果ethernet packet的类型是ARP并且payload可以被解析成ARP packet并且解析后的target ip是我自己，先将对应发送者的ip，ethernet还有当前时间记录在ARP_dictionary_中。
        1. 如果收到的是ARP request，调用send_ARP_reply()
        2. 如果收到的是ARP reply，将待发送的数据从data_wait_for_ARP_reply_取出来，并调用send_Ethernet_frame()
4. tick()：对wait_for_reply_里面所有的等待回复的ARP_query检查
    1. 如果front()的ARP_query超过RETRANSMIT_ARP_TIMER，需要将该query直接从wait_for_reply_中pop掉
        1. 如果在ARP_dictionary_里query里的ip已经被记录了并且没有超过UPDATE_DICTIONARY_TIMER，那么说明这一条query已经被回复了，什么都不需要做
        2. 否则说明query超时并且没有得到回复，需要将query中的arp包放入ready_to_send里面等待发送，并且将更新时间后的query放到wait_for_reply_中。
5. send_ARP_request()：
    1. 如果已经在RETRANSMIT_ARP_TIMER内发送过同一个request(data_wait_for_ARP_reply_中待发送数据存储的target ip已经存在了)，那么就直接返回
    2. 否则broadcast一个ARP request(一个特殊的ethernet packet)并等待想要的IP来回复它对应的ethernet address，将封装好的ethernet packet放到ready_to_send里面等待发送，同时将产生的ARP query放到wait_for_reply_里等待回复超时后重发
6. send_Ethernet_frame()：根据IP和ethernet address添加ethernet header，产生ethernet packet，然后放到ready_to_send里面等待发送
7. send_ARP_reply()：发送一个ARP reply packet，将自己的IP address和ethernet address放在ARP包里，并将封装好的ethernet packet放到ready_to_send里面等待发送

```cpp
NetworkInterface::NetworkInterface( const EthernetAddress& ethernet_address, const Address& ip_address )
  : ethernet_address_( ethernet_address ), ip_address_( ip_address )
{
  cerr << "DEBUG: Network interface has Ethernet address " << to_string( ethernet_address_ ) << " and IP address "
       << ip_address.ip() << "\n";
}

// dgram: the IPv4 datagram to be sent
// next_hop: the IP address of the interface to send it to (typically a router or default gateway, but
// may also be another host if directly connected to the same network as the destination)

// Note: the Address type can be converted to a uint32_t (raw 32-bit IP address) by using the
// Address::ipv4_numeric() method.
void NetworkInterface::send_datagram( const InternetDatagram& dgram, const Address& next_hop )
{
  // If the destination Ethernet address is already known, send it right away. Create
  // an Ethernet frame (with type = EthernetHeader::TYPE IPv4), set the payload to
  // be the serialized datagram, and set the source and destination addresses.
  IPAddr32 next_hop_ip = next_hop.ipv4_numeric();
  if ( ARP_dictionary_.find( next_hop_ip ) != ARP_dictionary_.end()
       && cur_ms - ARP_dictionary_[next_hop_ip].second <= UPDATE_DICTIONARY_TIMER ) {
    send_Ethernet_frame( dgram, next_hop_ip );
  }
  // If the destination Ethernet address is unknown, broadcast an ARP request for the
  // next hop’s Ethernet address, and queue the IP datagram so it can be sent after
  // the ARP reply is received.
  else {
    send_ARP_request( dgram, next_hop_ip );
  }
}

void NetworkInterface::send_Ethernet_frame( const InternetDatagram& dgram, const IPAddr32& next_hop_ip )
{
  EthernetFrame frame;
  frame.header.type = EthernetHeader::TYPE_IPv4;
  frame.header.src = ethernet_address_;
  frame.header.dst = ARP_dictionary_[next_hop_ip].first;
  frame.payload = serialize( dgram );
  ready_to_send_.push( frame );
}

void NetworkInterface::send_ARP_request( const InternetDatagram& dgram, const IPAddr32& next_hop_ip )
{
  // If already sent an ARP request, which means the data is waiting for ARP reply, do nothing
  if ( data_wait_for_ARP_reply_.find( next_hop_ip ) != data_wait_for_ARP_reply_.end() ) {
    return;
  }
  EthernetFrame frame;
  frame.header.type = EthernetHeader::TYPE_ARP;
  frame.header.src = ethernet_address_;
  frame.header.dst = ETHERNET_BROADCAST;
  ARPMessage arp;
  arp.opcode = ARPMessage::OPCODE_REQUEST;
  arp.sender_ethernet_address = ethernet_address_;
  arp.target_ethernet_address = { 0, 0, 0, 0, 0, 0 };
  arp.sender_ip_address = ip_address_.ipv4_numeric();
  arp.target_ip_address = next_hop_ip;
  frame.payload = serialize( arp );
  ready_to_send_.push( frame );
  wait_for_reply_.push( ARP_query { next_hop_ip, frame, cur_ms } );
  data_wait_for_ARP_reply_[next_hop_ip] = dgram;
}

// frame: the incoming Ethernet frame
// This method is called when an Ethernet frame arrives from the network. The code
// should ignore any frames not destined for the network interface (meaning, the Ethernet
// destination is either the broadcast address or the interface’s own Ethernet address
// stored in the ethernet address member variable).
optional<InternetDatagram> NetworkInterface::recv_frame( const EthernetFrame& frame )
{
  //   If the inbound frame is IPv4, parse the payload as an InternetDatagram and,
  // if successful (meaning the parse() method returned true), return the resulting
  // InternetDatagram to the caller.
  if ( frame.header.type == EthernetHeader::TYPE_IPv4 ) {
    InternetDatagram dgram;
    if ( frame.header.dst == ethernet_address_ && parse( dgram, frame.payload ) ) {
      return dgram;
    }
  }
  //   If the inbound frame is ARP, parse the payload as an ARPMessage and, if successful,
  // remember the mapping between the sender’s IP address and Ethernet address for
  // 30 seconds. (Learn mappings from both requests and replies.) In addition, if it’s
  // an ARP request asking for our IP address, send an appropriate ARP reply
  else if ( frame.header.type == EthernetHeader::TYPE_ARP ) {
    ARPMessage arp;
    if ( parse( arp, frame.payload ) && arp.target_ip_address == ip_address_.ipv4_numeric() ) {
      ARP_dictionary_[arp.sender_ip_address] = make_pair( arp.sender_ethernet_address, cur_ms );
      if ( arp.opcode == ARPMessage::OPCODE_REQUEST ) {
        send_ARP_reply( arp.sender_ip_address, arp.sender_ethernet_address );
      } else if ( arp.opcode == ARPMessage::OPCODE_REPLY ) {
        IPAddr32 target_ip = arp.sender_ip_address;
        if ( data_wait_for_ARP_reply_.find( target_ip ) != data_wait_for_ARP_reply_.end() ) {
          send_Ethernet_frame( data_wait_for_ARP_reply_[target_ip], target_ip );
          data_wait_for_ARP_reply_.erase( target_ip );
        }
      }
    }
  }
  return nullopt;
}

void NetworkInterface::send_ARP_reply( const IPAddr32& target_ip, const EthernetAddress& target_ethernet_address )
{
  EthernetFrame frame;
  frame.header.type = EthernetHeader::TYPE_ARP;
  frame.header.src = ethernet_address_;
  frame.header.dst = target_ethernet_address;
  ARPMessage arp;
  arp.opcode = ARPMessage::OPCODE_REPLY;
  arp.sender_ethernet_address = ethernet_address_;
  arp.target_ethernet_address = target_ethernet_address;
  arp.sender_ip_address = ip_address_.ipv4_numeric();
  arp.target_ip_address = target_ip;
  frame.payload = serialize( arp );
  ready_to_send_.push( frame );
}

// ms_since_last_tick: the number of milliseconds since the last call to this method
// This is called as time passes. Expire any IP-to-Ethernet mappings that have expired
void NetworkInterface::tick( const size_t ms_since_last_tick )
{
  cur_ms += ms_since_last_tick;
  while ( !wait_for_reply_.empty() && cur_ms - wait_for_reply_.front().time > RETRANSMIT_ARP_TIMER ) {
    ARP_query query = wait_for_reply_.front();
    wait_for_reply_.pop();
    auto query_ip = query.ip;
    auto query_frame = query.frame;
    if ( ARP_dictionary_.find( query_ip ) != ARP_dictionary_.end()
         && cur_ms - ARP_dictionary_[query_ip].second <= UPDATE_DICTIONARY_TIMER ) {
      continue;
    } else {
      ready_to_send_.push( query_frame );
      wait_for_reply_.push( ARP_query { query_ip, query_frame, cur_ms } );
    }
  }
}

optional<EthernetFrame> NetworkInterface::maybe_send()
{
  if ( ready_to_send_.empty() ) {
    return nullopt;
  }
  EthernetFrame frame = ready_to_send_.front();
  ready_to_send_.pop();
  return frame;
}
```

## 3. Q&A

## 4. 测试

![Screenshot 2023-12-16 at 11.38.22 PM.png](CS144%E5%AE%9E%E9%AA%8C%E8%AE%B0%E5%BD%95%20c54a23d522b645d18d8868dafcdc442e/Screenshot_2023-12-16_at_11.38.22_PM.png)

## 5. 提交代码

老样子

# CheckPoint5

## 0. 背景故事

这次要设计的是一个IP Router。之前设计的Network Interface实现了可以将收到的IP datagram根据提供的next_hop_ip通过Link layer进行一次转发。

![Screenshot 2023-12-23 at 11.56.40 PM.png](CS144%E5%AE%9E%E9%AA%8C%E8%AE%B0%E5%BD%95%20c54a23d522b645d18d8868dafcdc442e/Screenshot_2023-12-23_at_11.56.40_PM.png)

这次的IP Router会包含多个Network Interface并将接收到/产生的IP datagram重新通过这些Network Interface来不断的转发给下一个Router，直到IP datagram到达目的地。IP Router主要有以下两点

1. 维护一个routing table
2. 对于每个Interface收到的IP datagram查找routing table，然后根据longest prefix match原则找到匹配上的action，由该action可以知道当前这个IP datagram的next_hop_ip(下一个router的ip)是多少，以及负责转发给next_hop_ip的端口是哪一个interface。然后通过该interface去转发该IP datagram。

## 1. 总览

这次要做的IP Router的结构就如下图所示，简而言之就是包含多个Network Interface，因此就可以实现多个Network之间IP datagram的接收和转发。这些Network Interface都是Router自带的，就和之前的实现一样，每个Interface都有自己的IP address和ethernet address，并且和下一个hop的一个或多个Network Interface相连接。对于Router而言，每个Network Interface除了可以转发包，还可以接收包，为了实现转发，还缺一个routing table。Routing table的作用就是将每一个接收到的包，根据其dst_ip与routing table中的每一个entry进行longest prefix match，最终与其中的一个entry匹配上。然后根据那个entry提供的interface_number和next_hop_ip委托给某一个interface来进行转发。

至于每一个entry中的IP prefix和action的关系是怎么得到的，这是由一些算法(RIP,OSPF等)决定的，这次project不需要去实现那些算法，会直接给出<IP_prefix,action>用来构建routing table。

![Screenshot 2023-12-23 at 11.59.26 PM.png](CS144%E5%AE%9E%E9%AA%8C%E8%AE%B0%E5%BD%95%20c54a23d522b645d18d8868dafcdc442e/Screenshot_2023-12-23_at_11.59.26_PM.png)

## 2. 拉代码

```cpp
git fetch --all
git merge origin/check5-startercode
git merge upstream/check5-startercode
cmake -S . -B build
cmake --build build
```

## 3. Router实现

Router的实现其实很简单。首先观察到源码对上次实现的NetworkInterface进行了一次封装，recv_frame()函数不再直接返回IP datagram，而是将其存储在对应的Interface的queue中，以便由Router后续取出进行转发。想要取出接收到的IP datagram，需要调用maybe_receive()，如果返回值是空值，则表示该Interface接收到的IP datagram已经全部转发完毕。如果返回值是一个IP datagram，那么由Router对其进行转发。

想要根据routing table进行转发，那么首先要有routing table。routing table可以用一个vector来表示，而其中的每一条entry可以自己创造一个数据结构table entry来表示：

```cpp
// A routing table entry
struct tableEntry
{
  uint32_t route_prefix_;
  uint8_t prefix_length_;
  std::optional<Address> next_hop_;
  size_t interface_num_;
  uint32_t mask_;
  tableEntry( uint32_t route_prefix, uint8_t prefix_length, std::optional<Address> next_hop, size_t interface_num )
    : route_prefix_( route_prefix )
    , prefix_length_( prefix_length )
    , next_hop_( next_hop )
    , interface_num_( interface_num )
    , mask_( prefix_length_ == 0 ? 0 : ( 0xFFFFFFFF << ( 32 - prefix_length_ ) ) )
  {}
};
```

其中mask_是用来方便进行longest prefix match算法。

### Router类API实现

**Router::add_route()：**就初始化table entry，然后push到table(感觉最大的意义是教会了我怎么不用gdb打印debug信息，原来要用cerr输出😂)

```cpp
void Router::add_route( const uint32_t route_prefix,
                        const uint8_t prefix_length,
                        const optional<Address> next_hop,
                        const size_t interface_num )
{
  cerr << "DEBUG: adding route " << Address::from_ipv4_numeric( route_prefix ).ip() << "/"
       << static_cast<int>( prefix_length ) << " => " << ( next_hop.has_value() ? next_hop->ip() : "(direct)" )
       << " on interface " << interface_num << "\n";

  table_.push_back( tableEntry( route_prefix, prefix_length, next_hop, interface_num ) );
}
```

**Router::route()：**遍历每一个interface，对每个interface中的IP datagram调用**match_then_send()**

```cpp
void Router::route()
{
  for ( auto& interface : interfaces_ ) {
    while ( true ) {
      auto datagram = interface.maybe_receive();
      if ( !datagram.has_value() ) {
        break;
      }
      match_then_send( datagram.value() );
    }
  }
}
```

**void Router::match_then_send()：**如果当前的IP datagram的ttl已经小于1了，直接drop掉。因为就算为1，经过一次转发，降为0，还是要被drop掉。接着对table里的每个entry进行longest prefix match，只要dst_ip&mast==route_prefix，就相当于和当前entry匹配上了，接下来要考虑是不是longest prefix就看prefix_length是不是比之前匹配上的长。如果是第一次匹配上，或者是最新的最长匹配，那么更新结果。

如果没有匹配上，直接drop掉。如果匹配上了(即使route_prefix==0也是匹配上，default next hop)，那么根据匹配上entry的next hop是否有值来决定next hop ip。如果没有值，表示当前router已经是连接destination的router了，那么下一步就是直接发送给destination，next_hop==dst。否则还需要下一个router进行转发，next_hop就是entry里保存的下一个router的ip address。

知道了next_hop，那么就需要把ttl减一准备委托给对应的interface进行转发。转发前记得用compute_checksum()来根据更新后的ttl计算header中新的checksum！！！

```cpp
void Router::match_then_send( InternetDatagram& datagram )
{
  // Find the route with the longest prefix that matches the datagram's destination address
  // TTL is less than 1, drop the packet
  if ( datagram.header.ttl <= 1 )
    return;
  // Start matching process
  bool matched = false;
  size_t matched_idx = 0;
  for ( size_t i = 0; i < table_.size(); i++ ) {
    // Matched
    if ( ( ( datagram.header.dst & table_[i].mask_ ) == table_[i].route_prefix_ )
         && ( ( !matched ) || ( table_[i].prefix_length_ > table_[matched_idx].prefix_length_ ) ) ) {
      matched = true;
      matched_idx = i;
    }
  }
  // Not matched, drop the packet
  if ( !matched )
    return;
  datagram.header.ttl--;
  datagram.header.compute_checksum();
  Address dst_addr = table_[matched_idx].next_hop_.has_value() ? table_[matched_idx].next_hop_.value()
                                                               : Address::from_ipv4_numeric( datagram.header.dst );
  interface( table_[matched_idx].interface_num_ ).send_datagram( datagram, dst_addr );
}
```

## 4. 测试

![Screenshot 2023-12-24 at 12.52.56 AM.png](CS144%E5%AE%9E%E9%AA%8C%E8%AE%B0%E5%BD%95%20c54a23d522b645d18d8868dafcdc442e/Screenshot_2023-12-24_at_12.52.56_AM.png)

## 5. Q&A

## 6. 提交代码

# CheckPoint6

没啥好说的，如果之前每个component都做的差不多，这一部分基本就是进行一些双端通讯和互传文件测试。

如果之前做的不太好，这一部分就是疯狂debug。

然后我这边build不成功，需要改动/workspaces/codespaces-blank/minnow/util/CMakeLists.txt
，来将target和pthread库link。

```cpp

file(GLOB LIB_SOURCES "*.cc")

add_library(util_debug STATIC ${LIB_SOURCES})
target_link_libraries(util_debug pthread)

add_library(util_sanitized EXCLUDE_FROM_ALL STATIC ${LIB_SOURCES})
target_compile_options(util_sanitized PUBLIC ${SANITIZING_FLAGS})
target_link_libraries(util_sanitized pthread)

add_library(util_optimized EXCLUDE_FROM_ALL STATIC ${LIB_SOURCES})
target_compile_options(util_optimized PUBLIC "-O2")

target_link_libraries(util_optimized pthread)
```

# CheckPoint7

网络测试，计算一些网络的性能。

完结，🎉！