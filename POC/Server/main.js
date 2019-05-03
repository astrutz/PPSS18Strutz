const express = require('express');
const axios = require('axios');
var myParser = require("body-parser");
let savedIssues = [];
let app = express();
app.use(myParser.json({extended: true}));

app.get('/:issue', function (req, res) {

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
        });
});

app.put('/status/:issue', function (req, res) {
    let transitionId = increaseStatus(req.params.issue);
    axios.post('https://jira.kernarea.de/rest/api/2/issue/'+req.params.issue+'/transitions?expand=transitions.fields', {
        auth: {
            username: 'astrutz',
            password: 'Pitesti12345!'
        },
        data: {
            "transition": {
                "id": transitionId //TODO: TEST THIS WITH JIRA API
            }
        }
    }).then(function (response) {

    }).catch(function (error) {

    }).then(function () {
        res.sendStatus(200); //TODO: Send the new status string to change the card
    });
});

app.listen(1339, function () {
    console.log('Server listening on port 3000!');
});

function filterData(data, abbreviation) {
    let filteredData = {};
    if (data["issuetype"]["name"].toLowerCase() === "story") {
        filteredData.name = data["summary"];
        filteredData.description = data["description"];
        filteredData.abbreviation = abbreviation;
        filteredData.assignee = data["assignee"]["displayName"];
        filteredData.status = data["status"]["name"];
        filteredData.issuetype = "Story";
    } else if (data["issuetype"]["name"].toLowerCase() === "unteraufgabe") {
        filteredData.name = data["summary"];
        filteredData.abbreviation = abbreviation;
        filteredData.assignee = data["assignee"]["displayName"];
        filteredData.status = data["status"]["name"];
        filteredData.issuetype = "Unteraufgabe";
    } else {
        console.error("Issuetype = " + data["issuetype"]["name"]);
        filteredData.name = "Error!";
    }
    updateLocalIssues(filteredData);
    return filteredData;
}

function updateLocalIssues(issue) {
    let found = false;
    for (let i in savedIssues) {
        if (savedIssues[i]["abbreviation"] === issue ["abbreviation"]) {
            savedIssues[i] = issue;
            found = true;
        }
    }
    if (!found) {
        savedIssues.push(issue);

    }
}

function increaseStatus(abbreviation) {
    for (let i in savedIssues) {
        if (savedIssues[i]["abbreviation"] === abbreviation) {
            switch (savedIssues[i]["status"]) {
                case "Aufgaben":
                    savedIssues[i]["status"] = "Wird Ausgeführt";
                    return 21;
                case "Wird Ausgeführt":
                    savedIssues[i]["status"] = "Fertig";
                    return 31;
                case "Fertig":
                    savedIssues[i]["status"] = "Aufgaben";
                    return 11;
                default:
                    break;
            }
        }
    }
}