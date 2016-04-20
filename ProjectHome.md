技术博客：http://aquester.cublog.cn
此路不通，搬家了：https://github.com/eyjian/mooon

商业友好的开源mooon将致力于为创业者提供一个最佳分布式后台，高性能为节省成倍的机器成本，强大的可运营性大量减少运营成本，简单的编程界面大幅缩减开发成本，高质量的文档让上手无后顾之忧。这一切你不需要任何付出，即能随手可得。


理念：类库、平台不但要追求高性能、可扩展等，更要注重用户体验。

项目层次划分：

1.基础类库

什么是类库？在mooon中是指不包含复杂逻辑，通常也不会创建线程的，仅提供对某类功能面向对象化的C++包装。

mooon基础类库成员，包括util、sys和net三个基础类库，以及plugin\_mysql和plugin\_tinyxml插件。


2.公共组件

什么是组件？组件基于类库，通常是包含了逻辑的、需要记录日志的、内部具有线程的功能模块。

mooon公共组件成员，包括agent、dispatcher、cluster\_util、observer、server和http\_parser等

3.分布式应用平台

包括scheduler、namingservice和service等，提供有状态和线程安全的Session，以及无状态的Object，支持多方会话。基于公共组件，提供高性能和具备容灾能力的消息和文件分发功能。

4.Web应用

http-stress:一个类似于ab的HTTP压力测试工具，相比ab支持对URL同时进行测试。


5.海量应用平台

计划在分布式应用平台之上，打造一个优于 MapReduce 架构的大规模数据计算平台，支持状态化的复杂数据计算。


mooon相对其它平台的优点：

1).基于用户体验的接口和文档

2).高性能通讯平台，同时支持消息和文件

3).分层结构，功能模块组件化，具备高可扩展性

4).丰富的监控和状态数据，具备强的可测试性和可观察性

5).系统透明，易于运维

6).支持多方会话等



项目开发论坛：

http://bbs.hadoopor.com/index.php?gid=67



也可关注博客：

http://blog.chinaunix.net/uid/20682147/frmd/3967.html