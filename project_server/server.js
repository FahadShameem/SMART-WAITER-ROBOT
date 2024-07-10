const express = require('express');
const path = require('path');
const app = express();
const botcomm =express();

app.use(express.urlencoded({ extended: true }));
botcomm.use(express.urlencoded({ extended: true }));
app.use(express.static('public'));

let table_nos = [];

// Define a route for the root path
app.get('/', (req, res) => {

  // Send the HTML file as a response
  res.sendFile(path.join(__dirname, 'project.html'));
});



app.post('/submit', (req, res) => {

    console.log(req.body);
    if (req.body.tablenumber != '')
    {
      table_nos.push(req.body.tablenumber);
      console.log('table number:', table_nos[table_nos.length-1]);
    }
    console.log("ORDER QUEU : ",table_nos);

    
  res.sendFile(path.join(__dirname, 'success.html'));
  });


  botcomm.get('/', (req,res)=>{

    if (table_nos[0] != undefined )
    {
  
      res.send(table_nos[0]);
      console.log("from bottcomm : sended table.no to robot : ",table_nos[0]);
      table_nos.shift();
      console.log("ORDER QUEU : ",table_nos);


    }
    else
    {
      res.send("no data");
      console.log("from bottcom : no data");
    }
  });
  

// Start the server
const PORT = 3000;
const PORT2 = 4000;
app.listen(PORT, () => {
  console.log(`Server is running on http://localhost:${PORT}`);
});
botcomm.listen(PORT2, () => {
  console.log(`Server is running on http://localhost:${PORT2}`);
});