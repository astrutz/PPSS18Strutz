const express = require('express');
const axios = require('axios');
var myParser = require("body-parser");
let app = express();
app.use(myParser.json({extended: true}));
let counter;

app.get('/:issue', function (req, res) {

    counter ++;

    axios.get('https://jira.kernarea.de/rest/api/2/issue/' + req.params.issue, {
        auth: {
            username: 'astrutz',
            password: 'Pitesti12345!'
        }
    })
        .then(function (response) {
            console.log(response);
            res.send(JSON.stringify(response.data));
        })
        .catch(function (error) {
            console.log(error);
        })
        .then(function () {
            console.log("DONE in Versuch " + counter);
        });
});

app.listen(1339, function () {
    console.log('Server listening on port 3000!');
});