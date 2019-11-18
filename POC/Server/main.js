const express = require('express');
const axios = require('axios');
const fs = require('fs');
const jsdom = require('jsdom'); 
const {JSDOM} = jsdom;
var bodyparser = require("body-parser");
let activeUser = "null";
let app = express();
let issueId = '';
let PORT = process.env.PORT || 3000;
app.use(bodyparser.urlencoded({extended: true}));

app.put('/sprint/assign/', function (req, res) {
    let boardId = JSON.parse(fs.readFileSync('settings.json', 'UTF-8'))['boardId'];
    let sprints = [];
    let issues = [];
    let counter = 0;
    axios({
        method: 'get',
        url: 'https://jira.kernarea.de/rest/agile/1.0/board/' + boardId + '/sprint',
        auth: {
            username: 'astrutz',
            password: 'TarguMures56!'
        }
    }).then(function (response) {
        for (let i in response.data.values) {
            if (response.data.values[i]['state'] === 'active') {
                sprints.push(response.data.values[i]);
            }
        }
        for (let i in sprints) {
            axios({
                method: 'get',
                url: 'https://jira.kernarea.de/rest/agile/1.0/sprint/' + sprints[i]['id'] + '/issue/?jql=card_id is EMPTY AND type = Story',
                auth: {
                    username: 'astrutz',
                    password: 'TarguMures56!'
                }
            }).then(function (res) {
                for (let j in res.data['issues']) {
                    issues.push(res.data['issues'][j]['key']);
                }
                counter++;
            }).finally(function () {
                if (counter === sprints.length) {
                    let cardOverview = JSON.parse(fs.readFileSync('cardOverview.json', 'UTF-8'));
                    let emptyCards = [], emptyIssues, busyIssues;
                    for (let k in cardOverview) {
                        if (cardOverview[k]['abbreviation'] == null) {
                            emptyCards.push(cardOverview[k]);
                        }
                    }
                    emptyIssues = issues.slice(0, emptyCards.length);
                    busyIssues = issues.slice(emptyCards.length, issues.length);
                    for (let l in emptyIssues) {
                        let mixedString = emptyIssues[l] + ',' + emptyCards[l]['cardId'];
                        axios({
                            method: 'put',
                            url: 'http://localhost:' + PORT + '/direct/assign/' + mixedString
                        }).then(function (response) {
                        }).catch(function (error) {
                            console.log(error);
                        });
                    }
                    res.send(busyIssues);
                }
            });
        }
    });
});

app.put('/direct/assign/:mixedString', function (req, res) {
    let story = req.params.mixedString.split(',')[0];
    let card = req.params.mixedString.split(',')[1];
    axios({
        method: 'put',
        url: 'https://jira.kernarea.de/rest/api/2/issue/' + story,
        auth: {
            username: 'astrutz',
            password: 'TarguMures56!'
        },
        data: {
            "fields": {
                "customfield_14305": card
            }
        }
    }).then(function (response) {
        res.send(card);
    }).catch(function (error) {
        console.log(error);
    });
});

app.put('/sprint/deassign', function (req, res) {
    let cardOverview = JSON.parse(fs.readFileSync('cardOverview.json', 'UTF-8'));
    for (let i in cardOverview) {
        let cardId = cardOverview[i]['cardId'];
        axios({
            method: 'put',
            url: 'http://localhost:' + PORT + '/card/deassign/' + cardId
        }).then(function (response) {
        }).catch(function (error) {
            console.log(error);
        });
    }
    res.sendStatus(200);
});

app.post('/settings', function (req, res) {
    let settingsJson = req.body.settingsJson;
    fs.writeFileSync('settings.json', JSON.stringify(settingsJson), 'UTF-8');
    res.sendStatus(200);
});

app.get('/img/:title', function (req, res) {
    res.sendFile(__dirname + '/img/' + req.params.title);
});

app.get('/style.css', function (req, res) {
    res.send(fs.readFileSync('style.css', 'UTF-8'));
});

app.get('/card/overview', function (req, res) {
    res.send(renderCardOverview());
});

app.put('/card/assign/:storyAbbr', function (req, res) {
    let storyAbbreviation = req.params.storyAbbr;
    let cardId = getCardIdForStory();
    if (cardId != null) {
        axios({
            method: 'put',
            url: 'https://jira.kernarea.de/rest/api/2/issue/' + storyAbbreviation,
            auth: {
                username: 'astrutz',
                password: 'TarguMures56!'
            },
            data: {
                "fields": {
                    "customfield_14305": cardId
                }
            }
        }).then(function (response) {
            res.send(cardId);
        }).catch(function (error) {
            console.log(error);
        });
    } else {
        res.send(null);
    }
});

app.put('/card/deassign/:cardID', function (req, res) {
    let storyAbbreviation = findAbbreviationByCardId(req.params.cardID);
    if (storyAbbreviation != null) {
        axios({
            method: 'put',
            url: 'https://jira.kernarea.de/rest/api/2/issue/' + storyAbbreviation,
            auth: {
                username: 'astrutz',
                password: 'TarguMures56!'
            },
            data: {
                "fields": {
                    "customfield_14305": null
                }
            }
        }).then(function (response) {
            res.send(storyAbbreviation);
        }).catch(function (error) {
            console.log(error);
        });
    }
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
            if (activeUser !== "null") {
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
                    }
                ).then(function (response) {
                }).catch(function (error) {
                    console.error(error['response']['data']['errorMessages']);
                });
                let responseString = "Forwarding status of "
                    + req.params.issueId + " assigned to " + activeUser;
                res.send(responseString);
            } else {
                res.send("Forwarding status of " + req.params.issueId);
            }
        }
    );
});

app.put('/card/:cardId', function (req, res) {
    let issueID = findAbbreviationByCardId(req.params.cardId);
    if (issueID != null) {
        axios({
                method: 'put',
                url: 'http://localhost:' + PORT + '/status/' + issueID
            }
        ).then(function (response) {
            res.send(response['data']);
        }).catch(function (error) {
            console.error(error);
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
    var mqttOptions = {
        username: 'gzxwcdxc',
        password: 'iTeRc1cdW_RG'
    }; //cloud Usage

    var mqtt = require('mqtt');
    //var client = mqtt.connect('mqtt://192.168.178.52'); For localhost usage
    var client = mqtt.connect('mqtt://farmer.cloudmqtt.com:12744', mqttOptions); //Cloud Usage
    let intervall = JSON.parse(fs.readFileSync('settings.json', 'UTF-8'))['intervall'];
    client.on('connect', function () {
        console.log('E');
        setInterval(async function () {
            let cardsToUpdate = await updateCardOverview();
            for (let i in cardsToUpdate) {
                if (cardsToUpdate[i]['abbreviation'] != null) {
                    client.publish(cardsToUpdate[i]['cardId'].toString(), cardsToUpdate[i]['abbreviation']);
                    console.log('Message sent: ' + cardsToUpdate[i]['cardId'].toString() + ' with ' + cardsToUpdate[i]['abbreviation']);
                } else {
                    client.publish(cardsToUpdate[i]['cardId'].toString(), "");
                    console.log('Message sent: ' + cardsToUpdate[i]['cardId'].toString() + ' with empty message.');
                }
            }
        }, intervall);
    });
});

function getCardIdForStory() {
    let cardOverview = JSON.parse(fs.readFileSync('cardOverview.json', 'UTF-8'));
    for (let i in cardOverview) {
        if (cardOverview[i]['abbreviation'] === null) {
            return cardOverview[i]['cardId'];
        }
    }
    return null;
}

function findAbbreviationByCardId(cardId) {
    let cardOverview = JSON.parse(fs.readFileSync('cardOverview.json', 'UTF-8'));
    for (let i in cardOverview) {
        if (cardOverview[i]['cardId'] === cardId && cardOverview[i]['abbreviation'] != null) {
            return cardOverview[i]['abbreviation'];
        }
    }
    return null;
}

function filterData(data, abbreviation) {
    let rows = JSON.parse(fs.readFileSync("settings.json", "UTF-8"))['rows'];
    let filteredData = {};
    for (let i in rows) {
        switch (rows[i]) {
            case 'abbreviation':
                filteredData['row' + i] = abbreviation;
                break;
            case 'title':
                filteredData['row' + i] = data["summary"];
                break;
            case 'bar':
                filteredData['row' + i] = getProgressCount(data);
                filteredData['isBar'] = 1;
                break;
            case 'points':
                filteredData['row' + i] = data["customfield_10002"];
                filteredData['isBar'] = 0;
                break;
            case 'assignee':
                filteredData['row' + i] = data["assignee"]["displayName"];
                break;
            case 'null':
                filteredData['row' + i] = '';
                break;
        }
    }
    //filteredData.status = data["status"]["name"];
    if (data["issuetype"]["name"].toLowerCase() === "story") {
        //filteredData.issuetype = "Story";
    } else if (data["issuetype"]["name"].toLowerCase() === "unteraufgabe") {
        //filteredData.issuetype = "Unteraufgabe";
    } else {
        console.error("Issuetype = " + data["issuetype"]["name"]);
        filteredData.name = "Error!";
    }
    updateLocalIssues(filteredData);
    return filteredData;
}

function getProgressCount(data) {
    let doneCounter = 0;
    let subissues = data['subtasks'];
    for (let i in subissues) {
        if (subissues[i]['fields']['status']['name'] === "Fertig") {
            doneCounter++;
        }
    }
    if (subissues.length === 0) {
        return 0;
    } else {
        return Math.round((doneCounter / subissues.length) * 100);
    }
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
    let settingsList = JSON.parse(fs.readFileSync("settings.json", "UTF-8"));
    let overviewList = JSON.parse(fs.readFileSync("cardOverview.json", "UTF-8"));
    const dom = new JSDOM(fs.readFileSync("card_overview.html", "UTF-8"));
    const $ = (require('jquery'))(dom.window);
    if (overviewList.length > 0) {
        $('.cardOverview').append(renderOverviewTable(overviewList));
    } else {
        $('.cardOverview').append('Es sind keine Karten im System eingetragen');
    }
    $('input[name="intervallNumber"]').attr('value', settingsList['intervall'] / 1000);
    $('input[name="boardNumber"]').attr('value', settingsList['boardId']);
    for (let i in settingsList['rows']) {
        $('#inputRow' + i).find('option[value="' + settingsList['rows'][i] + '"]').attr('selected', 'selected');
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
    let minutes;
    if (beginDate.getMinutes() < 10) {
        minutes = '0' + beginDate.getMinutes().toString();
    } else {
        minutes = beginDate.getMinutes().toString();
    }
    let beginDateString = beginDate.getDay() + '.' + beginDate.getMonth() + '.' + beginDate.getFullYear() + ' ' + beginDate.getHours() + ':' + minutes;
    let endDateString = endDate.getDay() + '.' + endDate.getMonth() + '.' + endDate.getFullYear() + ' ' + endDate.getHours() + ':' + minutes;
    return historyEntry['abbreviation'] + ' von ' + beginDateString + ' bis ' + endDateString;
}

function updateCardOverview() {
    return new Promise((resolve, reject) => {
        let updatesCards = [];
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
                        if ((overviewList[i]['abbreviation'] !== issueList[j]['key']) || (overviewList[i]['progress'] !== getProgressCount(issueList[j]['fields']))) {
                            //A card will be updated with a new story!
                            if (overviewList[i]['abbreviation'] != null) {
                                let newHistoryItem = JSON.parse(JSON.stringify(overviewList[i]));
                                delete newHistoryItem['history'];
                                delete newHistoryItem['cardId'];
                                newHistoryItem['timestampEnd'] = Math.floor(Date.now() / 1000);
                                overviewList[i]['history'].push(newHistoryItem);
                            }
                            overviewList[i]['timestampBegin'] = Math.floor(Date.now() / 1000);
                            overviewList[i]['abbreviation'] = issueList[j]['key'];
                            overviewList[i]['name'] = issueList[j]['fields']['summary'];
                            if (issueList[j]['fields']['assignee'] === null) {
                                overviewList[i]['assignee'] = "";
                            } else {
                                overviewList[i]['assignee'] = issueList[j]['fields']['assignee']['displayName'];
                            }
                            overviewList[i]['status'] = issueList[j]['fields']['status']['name'];
                            overviewList[i]['issuetype'] = issueList[j]['fields']['issuetype']['name'];
                            overviewList[i]['progress'] = getProgressCount(issueList[j]['fields']);
                            let newUpdateItem = {};
                            newUpdateItem['name'] = overviewList[i]['name'];
                            newUpdateItem['assignee'] = overviewList[i]['assignee'];
                            newUpdateItem['abbreviation'] = overviewList[i]['abbreviation'];
                            newUpdateItem['status'] = overviewList[i]['status'];
                            newUpdateItem['issuetype'] = overviewList[i]['issuetype'];
                            newUpdateItem['cardId'] = overviewList[i]['cardId'];
                            updatesCards.push(newUpdateItem);
                            fs.writeFileSync('cardOverview.json', JSON.stringify(overviewList), 'UTF-8');
                        }
                    }
                }
                if (!found) {
                    if (overviewList[i]['abbreviation'] != null) {
                        let newHistoryItem = JSON.parse(JSON.stringify(overviewList[i]));
                        delete newHistoryItem['history'];
                        delete newHistoryItem['cardId'];
                        newHistoryItem['timestampEnd'] = Math.floor(Date.now() / 1000);
                        overviewList[i]['timestampBegin'] = null;
                        overviewList[i]['abbreviation'] = null;
                        overviewList[i]['name'] = null;
                        overviewList[i]['assignee'] = null;
                        overviewList[i]['status'] = null;
                        overviewList[i]['issuetype'] = null;
                        overviewList[i]['progress'] = 0;
                        let newUpdateItem = {};
                        newUpdateItem['name'] = overviewList[i]['name'];
                        newUpdateItem['assignee'] = overviewList[i]['assignee'];
                        newUpdateItem['abbreviation'] = overviewList[i]['abbreviation'];
                        newUpdateItem['status'] = overviewList[i]['status'];
                        newUpdateItem['issuetype'] = overviewList[i]['issuetype'];
                        newUpdateItem['cardId'] = overviewList[i]['cardId'];
                        updatesCards.push(newUpdateItem);
                        overviewList[i]['history'].push(newHistoryItem);
                        fs.writeFileSync('cardOverview.json', JSON.stringify(overviewList), 'UTF-8');
                    }
                }
            }
        }).catch(function (error) {
            console.log(error);
            reject(error);
        }).finally(function () {
            resolve(updatesCards);
        });
    });
}
