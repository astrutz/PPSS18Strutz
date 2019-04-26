const express = require('express');
const axios = require('axios');
var myParser = require("body-parser");
let app = express();
app.use(myParser.json({extended: true}));
let counter = 0;

app.get('/:issue', function (req, res) {

    counter ++;

    axios.get('https://jira.kernarea.de/rest/api/2/issue/' + req.params.issue, {
        auth: {
            username: 'astrutz',
            password: 'Pitesti12345!'
        }
    })
        .then(function (response) {
            console.log(response.data["fields"]);
            res.send(filterData(response.data["fields"], req.params.issue));
        })
        .catch(function (error) {
            console.log(error);
        })
        .then(function () {
            console.log("DONE " + counter);
        });
});

app.listen(3000, function () {
    console.log('Server listening on port 3000!');
});

function filterData(data, abbreviation){
    let filteredData = {};
    if(data["issuetype"]["name"].toLowerCase() === "story"){
        filteredData.name = data["summary"];
        filteredData.description = data["description"];
        filteredData.abbreviation = abbreviation;
        filteredData.assignee = data["assignee"]["displayName"];
        filteredData.status = data["status"]["name"];
        filteredData.issuetype = "Story";
    } else if(data["issuetype"]["name"].toLowerCase() === "unteraufgabe") { //TODO: Heißt das Unteraufgabe?
    //TODO: Filtered Data befüllen
    } else {
        console.error("Issuetype= " + data["issuetype"]["name"]);
        filteredData.name = "Error!";
    }
    return filteredData;
}