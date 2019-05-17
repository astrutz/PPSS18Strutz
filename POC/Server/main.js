const express = require('express');
const axios = require('axios');
const fs = require('fs');
var myParser = require("body-parser");
let activeUser = "null";
let app = express();
let PORT = process.env.port || 3000;
app.use(myParser.json({extended: true}));

app.get('/issue/:issue', function (req, res) {

    axios({
        method: 'get',
        url: 'https://jira.kernarea.de/rest/api/2/issue/' + req.params.issue,
        auth: {
            username: 'astrutz',
            password: 'Pitesti12345!'
        }
    })
        .then(function (response) {
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
    axios({
        method: 'post',
        url: 'https://jira.kernarea.de/rest/api/2/issue/' + req.params.issue + '/transitions?expand=transitions.fields',
        auth: {
            username: 'astrutz',
            password: 'Pitesti12345!'
        },
        data: {
            transition: {
                "id": transitionId
            }
        }
    }).then(function (response) {

    }).catch(function (error) {
        console.error(error['response']['data']['errorMessages']);
    }).then(function () {
        if (activeUser !== "null") {

            axios({
                method: 'put',
                url: 'https://jira.kernarea.de/rest/api/2/issue/' + req.params.issue,
                auth: {
                    username: 'astrutz',
                    password: 'Pitesti12345!'
                },
                data: {
                    "fields": {
                        "assignee": {
                            "name": activeUser
                        }
                    }
                }
            }).then(function (response) {

            }).catch(function (error) {
                console.error(error['response']['data']['errorMessages']);
            }).then(function () {

            });
        }
        let responseString = "Forwarding status of " + req.params.issue + " assigned to " + activeUser;
        console.log(responseString);
        res.send(responseString);
    });

});

app.put('/login/:id', async function (req, res) {
    activeUser = getUserById(req.params.id);
    let responseString = "User " + activeUser + " log in successfully";
    console.log(responseString);
    res.send(responseString);
    await startDaily();
});

app.get('/dailyStatus', function (req, res) {
    res.send(activeUser);
});

app.listen(PORT, function () {
    console.log('Server listening on port '+PORT+'!');
});

function filterData(data, abbreviation) {
    let filteredData = {};
    if (data["issuetype"]["name"].toLowerCase() === "story") {
        filteredData.name = data["summary"];
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
    let savedIssues = JSON.parse(getSavedIssues());
    for (let i in savedIssues) {
        if (savedIssues[i]["abbreviation"] === issue ["abbreviation"]) {
            savedIssues[i] = issue;
            found = true;
        }
    }
    if (!found) {
        savedIssues.push(issue);
    }
    setSavedIssues(savedIssues);
}

function increaseStatus(abbreviation) {
    let savedIssues = JSON.parse(getSavedIssues());
    for (let i in savedIssues) {
        if (savedIssues[i]["abbreviation"] === abbreviation) {
            switch (savedIssues[i]["status"]) {
                case "Aufgaben":
                    savedIssues[i]["status"] = "Wird Ausgeführt";
                    setSavedIssues(savedIssues);
                    return 21;
                case "Wird Ausgeführt":
                    savedIssues[i]["status"] = "Fertig";
                    setSavedIssues(savedIssues);
                    return 31;
                case "In Arbeit":
                    savedIssues[i]["status"] = "Fertig";
                    setSavedIssues(savedIssues);
                    return 31;
                case "Fertig":
                    savedIssues[i]["status"] = "Aufgaben";
                    setSavedIssues(savedIssues);
                    return 11;
                default:
                    break;
            }
        }
    }
}

function getUserById(id) {
    let userMap = JSON.parse(fs.readFileSync("rfidUser.json", 'UTF8'));
    for (let i in userMap['user']) {
        if (userMap['user'][i]['id'] === id) {
            return userMap['user'][i]['name'];
        }
    }
}

function getSavedIssues() {
    return fs.readFileSync("savedIssues.json", 'UTF8');
}

function setSavedIssues(issues){
    fs.writeFileSync('savedIssues.json', JSON.stringify(issues), 'UTF-8');
}

async function startDaily(){
    return new Promise(function() {
        setTimeout(function() {
            activeUser = "null";
        }, 600000);
    });
}