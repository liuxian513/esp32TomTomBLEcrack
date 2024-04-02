const express = require('express');
const bodyParser = require('body-parser');

const app = express();
const PORT = 3000; // 可以更改为任何未使用的端口

// 存储从ESP32接收到的所有数据
let allDataFromESP32 = [];
let rawData = '';

app.use(bodyParser.json());

// 接收ESP32发送的数据并添加到数据数组中
app.post('/data', (req, res) => {
    console.log('Received data from ESP32:');
    console.log(req.body); // 打印从ESP32接收到的数据
    
	//贮存接受到的数据
	rawData = req.body;
    // 获取北京时间
    const now = new Date(); // 获取当前UTC时间
    const beijingTime = new Date(now.getTime() + 8 * 60 * 60 * 1000); // 转换为北京时间（UTC+8）

    // 将接收到的新数据以及北京时间添加到数组中
    allDataFromESP32.push({
        timestamp: beijingTime.toISOString(), // 转换为ISO格式的字符串
        data: req.body
    });

    res.status(200).send('Data received successfully');
});

// 提供一个网页，显示从ESP32接收到的所有数据，但以倒序方式显示
app.get('/display', (req, res) => {
    // 创建HTML字符串以显示所有数据
    let html = '<h1>All Data from ESP32 (Latest First)</h1><ul>';
    // 创建一个数据的副本并将其反转，然后遍历所有数据，并将每个数据项添加到HTML列表中
    let dataInReverse = [...allDataFromESP32].reverse(); // 复制并反转数组
    dataInReverse.forEach(item => {
        // 显示每一条数据及其北京时间戳
        html += `<li>${item.timestamp}: ${JSON.stringify(item.data)}</li>`; 
    });
    html += '</ul>';
    res.send(html);
});


app.get('/rawdata', (req, res) => {
    res.status(200).send(rawData);
});



app.listen(PORT, () => {
    console.log(`Server is running on http://localhost:${PORT}`);
});
