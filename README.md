# 使用esp32破解钉钉蓝牙打卡

此代码是用于破解钉钉动态蓝牙打卡

需要使用：2个esp32 1台vps服务器（如果你有公网ip的话就不需要） esp32要求联网

原理是设置一个监听esp32在办公室实时获取打卡机的rawdata，然后在办公室的esp32通过wifi将rawdata传送到服务器，另外一个esp32通过手机热点从服务器获取rawdata
然后广播蓝牙rawdata

作者能力有限，如果有bug请大家提出
