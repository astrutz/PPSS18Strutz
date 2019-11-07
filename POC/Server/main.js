const express = require('express');
const axios = require('axios');
const fs = require('fs');
const jsdom = require('jsdom');
const {JSDOM} = jsdom;
var myParser = require("body-parser");
let activeUser = "null";
let app = express();
let issueId = '';
let PORT = process.env.PORT || 3000;
app.use(myParser.json({extended: true}));

app.get('/img/:title', function (req, res) {
    res.sendFile(__dirname + '/img/' + req.params.title);
});

app.get('/style.css', function (req, res) {
    res.send(fs.readFileSync('style.css', 'UTF-8'));
});

app.get('/card/overview', function (req, res) {
    res.send(renderCardOverview());
});

app.get('/card/issue/:cardId', function (req, res) {
    axios({
        method: 'get',
        url: 'https://jira.kernarea.de/rest/api/2/search?jql=card_id~' + req.params.cardId,
        auth: {
            username: 'astrutz',
            password: 'TarguMures56!'
        }
    }).then(function (response) {
        issueId = response.data['issues'][0]['key'];
        axios({
            method: 'get',
            url: 'https://jira.kernarea.de/rest/api/2/issue/' + issueId,
            auth: {
                username: 'astrutz',
                password: 'TarguMures56!'
            }
        }).then(function (response) {
            res.send(filterData(response.data["fields"], issueId));
        });
    }).catch(function (error) {
        console.log(error);
    });
});

app.get('/issue/:issueId', function (req, res) {
    axios({
        method: 'get',
        url: 'https://jira.kernarea.de/rest/api/2/issue/' + req.params.issueId,
        auth: {
            username: 'astrutz',
            password: 'TarguMures56!'
        }
    }).then(function (response) {
        res.send(filterData(response.data["fields"], req.params.issueId));
    }).catch(function (error) {
        console.log(error);
    });
});

app.put('/status/:issueId', function (req, res) {
    if (activeUser !== "null") {
        let transitionId = increaseStatus(req.params.issueId);
        axios({
            method: 'post',
            url: 'https://jira.kernarea.de/rest/api/2/issue/'
                + req.params.issueId + '/transitions?expand=transitions.fields',
            auth: {
                username: 'astrutz',
                password: 'TarguMures56!'
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
            axios({
                method: 'put',
                url: 'https://jira.kernarea.de/rest/api/2/issue/' + req.params.issueId,
                auth: {
                    username: 'astrutz',
                    password: 'TarguMures56!'
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
            });
            let responseString = "Forwarding status of "
                + req.params.issueId + " assigned to " + activeUser;
            res.send(responseString);
        });
    }
});

app.put('/login/:userId', async function (req, res) {
    activeUser = getUserById(req.params.userId);
    let responseString = "User " + activeUser + " logged in successfully";
    res.send(responseString);
    await startDaily();
});

app.put('/logoff', function (req, res) {
    let responseString = "User " + activeUser + " logged off successfully";
    activeUser = "null";
    res.send(responseString);
});

app.get('/dailyStatus', function (req, res) {
    res.send(activeUser);
});

app.listen(PORT, function () {
    console.log('Server listening on port ' + PORT + '!');
    updateCardOverview();
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

function getUserById(userId) {
    let userMap = JSON.parse(fs.readFileSync("rfidUser.json", 'UTF8'));
    for (let i in userMap['user']) {
        if (userMap['user'][i]['id'] === userId) {
            return userMap['user'][i]['name'];
        }
    }
}

function getSavedIssues() {
    return fs.readFileSync("savedIssues.json", 'UTF8');
}

function setSavedIssues(issues) {
    fs.writeFileSync('savedIssues.json', JSON.stringify(issues), 'UTF-8');
}

async function startDaily() {
    return new Promise(function () {
        setTimeout(function () {
            activeUser = "null";
        }, 600000);
    });
}

function renderCardOverview() {
    let overviewList = JSON.parse(fs.readFileSync("cardOverview.json", "UTF-8"));
    const dom = new JSDOM(fs.readFileSync("card_overview.html", "UTF-8"));
    const $ = (require('jquery'))(dom.window);
    if (overviewList.length > 0) {
        $('.container').append(renderOverviewTable(overviewList));
    } else {
        $('.container').append('Es sind keine Karten im System eingetragen');
    }
    return $("html").html();
}

function renderOverviewTable(overviewList) {
    let table = '';
    table += '<table class="table-hover">' + '<thead></thead><tbody>' + getCardList(overviewList) + '</tbody>' + '</table>';
    return table;
}

function getCardList(overviewList) {
    let list = '';
    for (let i in overviewList) {
        let listItem = overviewList[i];
        let icon = '<img class="accordionIcon" id="accordionIcon' + i + '" src="/img/baseline-keyboard_arrow_down-24px.svg" alt="SVG mit img Tag laden">';
        list += '<tr onclick="moveAccordion(' + i + ')"><td id="card' + i + '">' + getCardOverviewTitle(listItem) + icon + '</td></tr>';
        list += '<tr class="accordionContent" id="content' + i + '">' + '<td>' + renderHistory(listItem['history']) + '</td>' + '</tr>'
    }
    return list;
}

function getCardOverviewTitle(listItem) {
    if (listItem['name'] != null) {
        return 'ID: ' + listItem['cardId'] + ' ist folgender Story zugewiesen: ' + listItem['abbreviation'] + ' ' + listItem['name'];
    } else {
        return 'ID: ' + listItem['cardId'] + ' ist verfügbar.';
    }
}

function renderHistory(historyItem) {
    let history = '';
    if (historyItem != null) {
        history += 'Bisherige Nutzungen: <ul>';
        for (let i in historyItem) {
            history += '<li>' + renderHistoryEntry(historyItem[i]) + '</li>';
        }
        history += '</ul>';
    } else {
        history = 'Keine vergangenen Nutzungen gefunden.'
    }
    return history;
}

function renderHistoryEntry(historyEntry) {
    let beginDate = new Date(historyEntry['timestampBegin'] * 1000);
    let endDate = new Date(historyEntry['timestampEnd'] * 1000);
    let beginDateString = beginDate.getDay() + '.' + beginDate.getMonth() + '.' + beginDate.getFullYear() + ' ' + beginDate.getHours() + ':' + beginDate.getMinutes();
    let endDateString = endDate.getDay() + '.' + endDate.getMonth() + '.' + endDate.getFullYear() + ' ' + endDate.getHours() + ':' + endDate.getMinutes();
    return historyEntry['abbreviation'] + ' von ' + beginDateString + ' bis ' + endDateString;
}

function updateCardOverview() {
    let overviewList = JSON.parse(fs.readFileSync('cardOverview.json', 'UTF-8'));
    axios({
        method: 'get',
        url: 'https://jira.kernarea.de/rest/api/2/search?jql=card_id!~x_id',
        auth: {
            username: 'astrutz',
            password: 'TarguMures56!'
        }
    }).then(function (response) {
        let issueList = response.data['issues'];
        for (let i in overviewList) {
            let found = false;
            for (let j in issueList) {
                //customfield_14305 is the card_id saved in JIRA issues
                if (overviewList[i]['cardId'] === issueList[j]['fields']['customfield_14305']) {
                    found = true;
                    if (overviewList[i]['abbreviation'] !== issueList[j]['key']) {
                        if (overviewList[i]['abbreviation'] != null) {
                            let newHistoryItem = JSON.parse(JSON.stringify(overviewList[i]));
                            delete newHistoryItem['history'];
                            delete newHistoryItem['cardId'];
                            newHistoryItem['timestampEnd'] = Date.now();
                            overviewList[i]['history'].push(newHistoryItem);
                        }
                        overviewList[i]['timestampBegin'] = Date.now();
                        overviewList[i]['abbreviation'] = issueList[j]['key'];
                        overviewList[i]['name'] = issueList[j]['fields']['summary'];
                        overviewList[i]['assignee'] = issueList[j]['fields']['assignee']['displayName'];
                        overviewList[i]['status'] = issueList[j]['fields']['status']['name'];
                        overviewList[i]['issuetype'] = issueList[j]['fields']['issuetype']['name'];
                        fs.writeFileSync('cardOverview.json', JSON.stringify(overviewList), 'UTF-8');
                    }
                }
            }
            if (!found) {
                if (overviewList[i]['abbreviation'] != null) {
                    let newHistoryItem = JSON.parse(JSON.stringify(overviewList[i]));
                    delete newHistoryItem['history'];
                    delete newHistoryItem['cardId'];
                    newHistoryItem['timestampEnd'] = Date.now();
                    overviewList[i]['timestampBegin'] = null;
                    overviewList[i]['abbreviation'] = null;
                    overviewList[i]['name'] = null;
                    overviewList[i]['assignee'] = null;
                    overviewList[i]['status'] = null;
                    overviewList[i]['issuetype'] = null;
                    overviewList[i]['history'].push(newHistoryItem);
                    fs.writeFileSync('cardOverview.json', JSON.stringify(overviewList), 'UTF-8');
                }
            }
        }

    }).catch(function (error) {
        console.log(error);
    });
    //TODO Call this every X Seconds, look for saved card_id's, check it with the JSON and update
}
