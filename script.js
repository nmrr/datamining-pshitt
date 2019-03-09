/*
MIT License

Copyright 2017 nmrr (https://github.com/nmrr)

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

var express = require('express');
var http = require('http');
var path = require('path');
var app = express();
var fs = require('fs');
var addonNodejs = require("./build/Release/addon.node");


app.set('etag', false);
app.set('x-powered-by', false);

var configJSON = undefined;

// Lecture du fichier de configuration
try
{
    var configFile = fs.readFileSync('./config.json', 'utf8');
    configJSON = JSON.parse(configFile);
}
catch(err)
{
    console.log(err);
    process.exit();
}

if (configJSON.geoIP === undefined || configJSON.sshLogs === undefined)
{
    console.log("bad config file");
    process.exit();
}

// Lecture des logs
addonNodejs.readfile(configJSON.geoIP, configJSON.sshLogs);

var logStream = fs.createWriteStream(__dirname + '/logs/log.txt', {'flags': 'a'});

function log(req)
{
    var data = {
        date: new Date(),
        method: req.method,
        ip: req.connection.remoteAddress,
        port: req.connection.remotePort,
        agent: req.headers['user-agent'],
        host: req.headers.host,
        connection: req.headers.connection,
        accept: req.headers.accept,
        referer: req.headers.referer,
        encoding: req.headers['accept-encoding'],
        language: req.headers['accept-language'],
        url: req._parsedUrl.pathname,
        fullurl: req._parsedUrl.path,
    };

    if (Object.keys(req.query).length > 0) data.data = req.query;

    return data;
}

app.use(function (req, res, next) {

    res.setHeader("Server", "MyWebServer");

    var data = log(req);
    logStream.write(JSON.stringify(data) + '\n');

    next();
});

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

app.use('/static', express.static('files'));

// Pour obtenir les limites temporelles des logs
app.get('/info/dateslimites.json', function (req, res) {

    addonNodejs.date(function (resultat)
    {
        res.writeHead(200, {
          'Content-Type': 'application/json',
          'Content-Length': resultat.length
        });
        res.end(resultat);
    });
});

// Pour avoir la liste des pays qui ont attaqué entre 2 dates
app.get('/info/pays.json', function (req, res) {

    var yearstart = 0, monthstart = 0, daystart = 0, yearend = 0, monthend = 0, dayend = 0;
    var suivant = 0;

    if (req.query.suivant !== undefined)
    {
        if (isNaN(req.query.suivant) === false)
        {
            suivant = parseInt(req.query.suivant);
        }
    }

    if (req.query.debut !== undefined)
    {
        var recuperation = "";
        var position = 0;

        //2019-12-04
        for (var i=0;i<req.query.debut.length;i++)
        {
            if (req.query.debut[i] !== '-') recuperation += req.query.debut[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            yearstart = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.debut.length;i++)
        {
            if (req.query.debut[i] !== '-') recuperation += req.query.debut[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            monthstart = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.debut.length;i++)
        {
            recuperation += req.query.debut[i];
        }

        if (isNaN(recuperation) === false)
        {
            daystart = parseInt(recuperation);
        }
    }

    if (req.query.fin !== undefined)
    {
        var recuperation = "";
        var position = 0;

        //2019-12-04
        for (var i=0;i<req.query.fin.length;i++)
        {
            if (req.query.fin[i] !== '-') recuperation += req.query.fin[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            yearend = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.fin.length;i++)
        {
            if (req.query.fin[i] !== '-') recuperation += req.query.fin[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            monthend = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.fin.length;i++)
        {
            recuperation += req.query.fin[i];
        }

        if (isNaN(recuperation) === false)
        {
            dayend = parseInt(recuperation);
        }
    }

    // Lancement de la méthode de l'addon
    addonNodejs.pays(yearstart, monthstart, daystart, yearend, monthend, dayend, suivant, function (resultat, err)
    {
        // Lecture des résultats
        if (err !== null)
        {
            if (err === 1)
            {
                res.status(404).send("beginning date");
            }
            else if (err === 2)
            {
                res.status(404).send("ending date");
            }
            else if (err === 3)
            {
                res.status(404).send("beginning date > ending date");
            }
            else
            {
                res.status(404).send("next, beginning date = ending date");
            }
        }
        else
        {
            // Envoi de données JSON
            res.writeHead(200, {
              'Content-Type': 'application/json',
              'Content-Length': resultat.length
            });
            res.end(resultat);
        }
    });
});

// Redirection de l'utilisateur vers la visualisation choisie (visualisation générale)
app.get('/form/visualisation.html', function (req, res) {


    var args = "?data=" + req.query.data + "&debut=" + req.query.debut + "&fin=" + req.query.fin + "&limite=" + req.query.limite;

    if (req.query.affichage === "linear")
    {
        res.redirect('/static/html/visualisation/linear.html' + args);
    }
    else if (req.query.affichage === "cercle")
    {
        res.redirect('/static/html/visualisation/cercle.html' + args);
    }
    else if (req.query.affichage === "cercle2")
    {
        res.redirect('/static/html/visualisation/cercle2.html' + args);
    }
    else if (req.query.affichage === "carte")
    {
        res.redirect('/static/html/visualisation/carte.html' + args);
    }
    else
    {
        res.redirect('/static/html/visualisation/tableau.html' + args);
    }
});

// Redirection de l'utilisateur vers la visualisation choisie (visualisation spécifique aux usernames, passwords)
app.get('/form/visualisation2.html', function (req, res) {

    var args = "?data=" + req.query.data + "&debut=" + req.query.debut + "&fin=" + req.query.fin + "&username=" + (req.query.username !== undefined ? req.query.username : "") + "&password=" + (req.query.password !== undefined ? req.query.password : "");

    if (req.query.affichage === "linear")
    {
        res.redirect('/static/html/visualisation2/linear.html' + args);
    }
    else if (req.query.affichage === "cercle")
    {
        res.redirect('/static/html/visualisation2/cercle.html' + args);
    }
    else if (req.query.affichage === "carte")
    {
        res.redirect('/static/html/visualisation2/carte.html' + args);
    }
    else
    {
        res.redirect('/static/html/visualisation2/tableau.html' + args);
    }
});

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Pour le graphe des attaques dans le temps (visualisation générale)
app.get('/generator/linear.csv', function (req, res) {

    var yearstart = 0, monthstart = 0, daystart = 0, yearend = 0, monthend = 0, dayend = 0;

    if (req.query.debut !== undefined)
    {
        var recuperation = "";
        var position = 0;

        //2019-12-04
        for (var i=0;i<req.query.debut.length;i++)
        {
            if (req.query.debut[i] !== '-') recuperation += req.query.debut[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            yearstart = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.debut.length;i++)
        {
            if (req.query.debut[i] !== '-') recuperation += req.query.debut[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            monthstart = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.debut.length;i++)
        {
            recuperation += req.query.debut[i];
        }

        if (isNaN(recuperation) === false)
        {
            daystart = parseInt(recuperation);
        }
    }

    if (req.query.fin !== undefined)
    {
        var recuperation = "";
        var position = 0;

        //2019-12-04
        for (var i=0;i<req.query.fin.length;i++)
        {
            if (req.query.fin[i] !== '-') recuperation += req.query.fin[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            yearend = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.fin.length;i++)
        {
            if (req.query.fin[i] !== '-') recuperation += req.query.fin[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            monthend = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.fin.length;i++)
        {
            recuperation += req.query.fin[i];
        }

        if (isNaN(recuperation) === false)
        {
            dayend = parseInt(recuperation);
        }
    }

    addonNodejs.linear(yearstart, monthstart, daystart, yearend, monthend, dayend, function (resultat, err, emptyData)
    {
        if (err !== null)
        {
            if (err === 1)
            {
                res.status(404).send("beginning date");
            }
            else if (err === 2)
            {
                res.status(404).send("ending date");
            }
            else
            {
                res.status(404).send("beginning date > ending date");
            }
        }
        else if (emptyData === false)
        {
            res.writeHead(200, {
              'Content-Type': 'text/csv',
              'Content-Length': resultat.length
            });
            res.end(resultat);
        }
        else
        {
            res.status(404).send("empty data");
        }
    });
});

// Pour le graphe des attaques dans le temps (visualisation spécifique aux usernames, passwords)
app.get('/generator/linear2.csv', function (req, res) {

    var yearstart = 0, monthstart = 0, daystart = 0, yearend = 0, monthend = 0, dayend = 0;
    var username = "", password = "";
    var data = -1;

    if (req.query.username !== undefined)
    {
        username = String(req.query.username);
    }

    if (req.query.password !== undefined)
    {
        password = String(req.query.password);
    }

    if (req.query.debut !== undefined)
    {
        var recuperation = "";
        var position = 0;

        //2019-12-04
        for (var i=0;i<req.query.debut.length;i++)
        {
            if (req.query.debut[i] !== '-') recuperation += req.query.debut[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            yearstart = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.debut.length;i++)
        {
            if (req.query.debut[i] !== '-') recuperation += req.query.debut[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            monthstart = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.debut.length;i++)
        {
            recuperation += req.query.debut[i];
        }

        if (isNaN(recuperation) === false)
        {
            daystart = parseInt(recuperation);
        }
    }

    if (req.query.fin !== undefined)
    {
        var recuperation = "";
        var position = 0;

        //2019-12-04
        for (var i=0;i<req.query.fin.length;i++)
        {
            if (req.query.fin[i] !== '-') recuperation += req.query.fin[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            yearend = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.fin.length;i++)
        {
            if (req.query.fin[i] !== '-') recuperation += req.query.fin[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            monthend = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.fin.length;i++)
        {
            recuperation += req.query.fin[i];
        }

        if (isNaN(recuperation) === false)
        {
            dayend = parseInt(recuperation);
        }
    }

    if (req.query.data !== undefined)
    {
        if (req.query.data === "username") data = 0;
        else if (req.query.data === "password") data = 1;
        else if (req.query.data === "usernamepassword") data = 2;
    }

    if (data !== -1)
    {
        addonNodejs.linear2(yearstart, monthstart, daystart, yearend, monthend, dayend, data, username, password, function (resultat, err, emptyData)
        {
            if (err !== null)
            {
                if (err === 1)
                {
                    res.status(404).send("beginning date");
                }
                else if (err === 2)
                {
                    res.status(404).send("ending date");
                }
                else if (err === 3)
                {
                    res.status(404).send("beginning date > ending date");
                }
                else if (err === 4)
                {
                    res.status(404).send("username not found");
                }
                else
                {
                    res.status(404).send("password not found");
                }
            }
            else if (emptyData === false)
            {
                res.writeHead(200, {
                  'Content-Type': 'text/csv',
                  'Content-Length': resultat.length
                });
                res.end(resultat);
            }
            else
            {
                res.status(404).send("empty data");
            }
        });
    }
    else
    {
        res.status(404).send("invalid args");
    }
});

// Graphe des attaques en fonction des pays
app.get('/generator/pays.json', function (req, res) {

    var yearstart = 0, monthstart = 0, daystart = 0, yearend = 0, monthend = 0, dayend = 0;
    var data = 0;
    var isolate = 0;
    var ip = 0

    if (req.query.data !== undefined)
    {
        if (req.query.data === "username")
        {
            data = 0;
        }
        else if (req.query.data === "password")
        {
            data = 1;
        }
        else if (req.query.data === "usernamepassword")
        {
            data = 2;
        }
    }

    if (req.query.isolate !== undefined)
    {
        if (req.query.isolate === "1") isolate = 1;
    }

    if (req.query.ip !== undefined)
    {
        if (req.query.ip === "1") ip = 1;
    }

    if (req.query.debut !== undefined)
    {
        var recuperation = "";
        var position = 0;

        //2019-12-04
        for (var i=0;i<req.query.debut.length;i++)
        {
            if (req.query.debut[i] !== '-') recuperation += req.query.debut[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            yearstart = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.debut.length;i++)
        {
            if (req.query.debut[i] !== '-') recuperation += req.query.debut[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            monthstart = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.debut.length;i++)
        {
            recuperation += req.query.debut[i];
        }

        if (isNaN(recuperation) === false)
        {
            daystart = parseInt(recuperation);
        }
    }

    if (req.query.fin !== undefined)
    {
        var recuperation = "";
        var position = 0;

        //2019-12-04
        for (var i=0;i<req.query.fin.length;i++)
        {
            if (req.query.fin[i] !== '-') recuperation += req.query.fin[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            yearend = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.fin.length;i++)
        {
            if (req.query.fin[i] !== '-') recuperation += req.query.fin[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            monthend = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.fin.length;i++)
        {
            recuperation += req.query.fin[i];
        }

        if (isNaN(recuperation) === false)
        {
            dayend = parseInt(recuperation);
        }
    }

    if (req.query.listepays !== undefined)
    {
        addonNodejs.paysGraphe(yearstart, monthstart, daystart, yearend, monthend, dayend, String(req.query.listepays), data, isolate, ip, function (resultat, err, emptyData)
        {
            if (err !== null)
            {
                if (err === 1)
                {
                    res.status(404).send("beginning date");
                }
                else if (err === 2)
                {
                    res.status(404).send("ending date");
                }
                else
                {
                    res.status(404).send("beginning date > ending date");
                }
            }
            else if (emptyData === false)
            {
                res.writeHead(200, {
                  'Content-Type': 'application/json',
                  'Content-Length': resultat.length
                });
                res.end(resultat);
            }
            else
            {
                res.status(404).send("empty data");
            }
        });
    }
    else
    {
        res.status(404).send("country list");
    }
});

// Graphe des anomalies
app.get('/generator/anomalie.json', function (req, res) {

    var yearstartapp = 0, monthstartapp = 0, daystartapp = 0, yearendapp = 0, monthendapp = 0, dayendapp = 0;
    var yearend = 0, monthend = 0, dayend = 0;
    var seuilapp = 10, seuil = 10;
    var data = 0;

    if (req.query.seuilapp !== undefined)
    {
        if (isNaN(req.query.seuilapp) === false)
        {
            seuilapp = parseInt(req.query.seuilapp);
            if (seuilapp <= 0) seuilapp = 10;
        }
    }

    if (req.query.seuil !== undefined)
    {
        if (isNaN(req.query.seuil) === false)
        {
            seuil = parseInt(req.query.seuil);
            if (seuil <= 0) seuil = 10;
        }
    }

    if (req.query.data === "username")
    {
        data = 0;
    }
    else if (req.query.data === "password")
    {
        data = 1;
    }
    else if (req.query.data === "usernamepassword")
    {
        data = 2;
    }

    if (req.query.debutapp !== undefined)
    {
        var recuperation = "";
        var position = 0;

        //2019-12-04
        for (var i=0;i<req.query.debutapp.length;i++)
        {
            if (req.query.debutapp[i] !== '-') recuperation += req.query.debutapp[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            yearstartapp = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.debutapp.length;i++)
        {
            if (req.query.debutapp[i] !== '-') recuperation += req.query.debutapp[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            monthstartapp = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.debutapp.length;i++)
        {
            recuperation += req.query.debutapp[i];
        }

        if (isNaN(recuperation) === false)
        {
            daystartapp = parseInt(recuperation);
        }
    }

    if (req.query.finapp !== undefined)
    {
        var recuperation = "";
        var position = 0;

        //2019-12-04
        for (var i=0;i<req.query.finapp.length;i++)
        {
            if (req.query.finapp[i] !== '-') recuperation += req.query.finapp[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            yearendapp = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.finapp.length;i++)
        {
            if (req.query.finapp[i] !== '-') recuperation += req.query.finapp[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            monthendapp = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.finapp.length;i++)
        {
            recuperation += req.query.finapp[i];
        }

        if (isNaN(recuperation) === false)
        {
            dayendapp = parseInt(recuperation);
        }
    }

    if (req.query.fin !== undefined)
    {
        var recuperation = "";
        var position = 0;

        //2019-12-04
        for (var i=0;i<req.query.fin.length;i++)
        {
            if (req.query.fin[i] !== '-') recuperation += req.query.fin[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            yearend = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.fin.length;i++)
        {
            if (req.query.fin[i] !== '-') recuperation += req.query.fin[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            monthend = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.fin.length;i++)
        {
            recuperation += req.query.fin[i];
        }

        if (isNaN(recuperation) === false)
        {
            dayend = parseInt(recuperation);
        }
    }

    if (req.query.listepaysapp !== undefined && req.query.listepays !== undefined)
    {
        addonNodejs.anomalieGraphe(yearstartapp, monthstartapp, daystartapp, yearendapp, monthendapp, dayendapp, yearend, monthend, dayend, String(req.query.listepaysapp), String(req.query.listepays), data, seuilapp, seuil, function (resultat, err, emptyData)
        {
            if (err !== null)
            {
                if (err === 1)
                {
                    res.status(404).send("beginning date app");
                }
                else if (err === 2)
                {
                    res.status(404).send("ending date app");
                }
                else if (err === 3)
                {
                    res.status(404).send("beginning date app > ending date app");
                }
                else if (err === 4)
                {
                    res.status(404).send("ending date app >= ending date");
                }
                else if (err === 5)
                {
                    res.status(404).send("ending date");
                }
                else if (err === 6)
                {
                    res.status(404).send("empty listepaysapp  or empty listepays");
                }
                else if (err === 7)
                {
                    res.status(404).send("empty dictionary app");
                }
                else if (err === 8)
                {
                    res.status(404).send("empty dictionary");
                }
                else
                {
                    res.status(404).send("unknown error : " + String(err));
                }
            }
            else if (emptyData === false)
            {
                res.writeHead(200, {
                  'Content-Type': 'application/json',
                  'Content-Length': resultat.length
                });
                res.end(resultat);
            }
            else
            {
                res.status(404).send("empty data");
            }
        });
    }
    else
    {
        res.status(404).send("country list");
    }
});

// Graphe des relations username/password (visualisation générale)
app.get('/generator/usernamepassword.json', function (req, res) {


    var yearstart = 0, monthstart = 0, daystart = 0, yearend = 0, monthend = 0, dayend = 0;
    var limite = 0;

    if (req.query.limite !== undefined)
    {
        if (isNaN(req.query.limite) === false)
        {
            limite = parseInt(req.query.limite);
        }
    }

    if (req.query.debut !== undefined)
    {
        var recuperation = "";
        var position = 0;

        //2019-12-04
        for (var i=0;i<req.query.debut.length;i++)
        {
            if (req.query.debut[i] !== '-') recuperation += req.query.debut[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            yearstart = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.debut.length;i++)
        {
            if (req.query.debut[i] !== '-') recuperation += req.query.debut[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            monthstart = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.debut.length;i++)
        {
            recuperation += req.query.debut[i];
        }

        if (isNaN(recuperation) === false)
        {
            daystart = parseInt(recuperation);
        }
    }

    if (req.query.fin !== undefined)
    {
        var recuperation = "";
        var position = 0;

        //2019-12-04
        for (var i=0;i<req.query.fin.length;i++)
        {
            if (req.query.fin[i] !== '-') recuperation += req.query.fin[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            yearend = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.fin.length;i++)
        {
            if (req.query.fin[i] !== '-') recuperation += req.query.fin[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            monthend = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.fin.length;i++)
        {
            recuperation += req.query.fin[i];
        }

        if (isNaN(recuperation) === false)
        {
            dayend = parseInt(recuperation);
        }
    }

    addonNodejs.visualisationUsernamePasswordCercle(yearstart, monthstart, daystart, yearend, monthend, dayend, limite, function (resultat, err, emptyData)
    {
        if (err !== null)
        {
            if (err === 1)
            {
                res.status(404).send("beginning date");
            }
            else if (err === 2)
            {
                res.status(404).send("ending date");
            }
            else
            {
                res.status(404).send("beginning date > ending date");
            }
        }
        else if (emptyData === false)
        {
            res.writeHead(200, {
              'Content-Type': 'application/json',
              'Content-Length': resultat.length
            });
            res.end(resultat);
        }
        else
        {
            res.status(404).send("empty data");
        }
    });
});

// Génération de la carte (visualisation générale)
app.get('/generator/carte.csv', function (req, res) {


    var yearstart = 0, monthstart = 0, daystart = 0, yearend = 0, monthend = 0, dayend = 0;
    var limite = 0;

    if (req.query.limite !== undefined)
    {
        if (isNaN(req.query.limite) === false)
        {
            limite = parseInt(req.query.limite);
        }
    }

    if (req.query.debut !== undefined)
    {
        var recuperation = "";
        var position = 0;

        //2019-12-04
        for (var i=0;i<req.query.debut.length;i++)
        {
            if (req.query.debut[i] !== '-') recuperation += req.query.debut[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            yearstart = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.debut.length;i++)
        {
            if (req.query.debut[i] !== '-') recuperation += req.query.debut[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            monthstart = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.debut.length;i++)
        {
            recuperation += req.query.debut[i];
        }

        if (isNaN(recuperation) === false)
        {
            daystart = parseInt(recuperation);
        }
    }

    if (req.query.fin !== undefined)
    {
        var recuperation = "";
        var position = 0;

        //2019-12-04
        for (var i=0;i<req.query.fin.length;i++)
        {
            if (req.query.fin[i] !== '-') recuperation += req.query.fin[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            yearend = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.fin.length;i++)
        {
            if (req.query.fin[i] !== '-') recuperation += req.query.fin[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            monthend = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.fin.length;i++)
        {
            recuperation += req.query.fin[i];
        }

        if (isNaN(recuperation) === false)
        {
            dayend = parseInt(recuperation);
        }
    }

    addonNodejs.visualisationCarte(yearstart, monthstart, daystart, yearend, monthend, dayend, limite, function (resultat, err, emptyData)
    {
        if (err !== null)
        {
            if (err === 1)
            {
                res.status(404).send("beginning date");
            }
            else if (err === 2)
            {
                res.status(404).send("ending date");
            }
            else
            {
                res.status(404).send("beginning date > ending date");
            }
        }
        else if (emptyData === false)
        {
            res.writeHead(200, {
              'Content-Type': 'text/csv',
              'Content-Length': resultat.length
            });
            res.end(resultat);
        }
        else
        {
            res.status(404).send("empty data");
        }
    });
});

// Visualisation des données générales, graphe en cercle et liste
app.get('/generator/visualisation.json', function (req, res) {


    var yearstart = 0, monthstart = 0, daystart = 0, yearend = 0, monthend = 0, dayend = 0;
    var limite = 0, data = -1;

    if (req.query.limite !== undefined)
    {
        if (isNaN(req.query.limite) === false)
        {
            limite = parseInt(req.query.limite);
        }
    }

    if (req.query.debut !== undefined)
    {
        var recuperation = "";
        var position = 0;

        //2019-12-04
        for (var i=0;i<req.query.debut.length;i++)
        {
            if (req.query.debut[i] !== '-') recuperation += req.query.debut[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            yearstart = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.debut.length;i++)
        {
            if (req.query.debut[i] !== '-') recuperation += req.query.debut[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            monthstart = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.debut.length;i++)
        {
            recuperation += req.query.debut[i];
        }

        if (isNaN(recuperation) === false)
        {
            daystart = parseInt(recuperation);
        }
    }

    if (req.query.fin !== undefined)
    {
        var recuperation = "";
        var position = 0;

        //2019-12-04
        for (var i=0;i<req.query.fin.length;i++)
        {
            if (req.query.fin[i] !== '-') recuperation += req.query.fin[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            yearend = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.fin.length;i++)
        {
            if (req.query.fin[i] !== '-') recuperation += req.query.fin[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            monthend = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.fin.length;i++)
        {
            recuperation += req.query.fin[i];
        }

        if (isNaN(recuperation) === false)
        {
            dayend = parseInt(recuperation);
        }
    }

    if (req.query.data !== undefined)
    {
        if (req.query.data === "country") data = 0;
        else if (req.query.data === "username") data = 1;
        else if (req.query.data === "password") data = 2;
        else if (req.query.data === "cipher") data = 3;
        else if (req.query.data === "mac") data = 4;
        else if (req.query.data === "port") data = 5;
        else if (req.query.data === "ip") data = 6;
        else if (req.query.data === "usernamepassword") data = 7;
        else if (req.query.data === "attaque") data = 8;
	    else if (req.query.data === "software") data = 9;
    }

    if (data !== -1)
    {
        console.log("data : " + data)

        console.log(req.query.data + ", " + limite + ", " + yearstart + " " + monthstart + " " + daystart + " -> " + yearend + " " + monthend + " " + dayend);

        // http://127.0.0.1:8888/country.json?yearstart=2016&monthstart=1&daystart=1&yearend=2016&monthend=2&dayend=1
        // http://127.0.0.1:8888/static/index.html?yearstart=2016&monthstart=1&daystart=1&yearend=2016&monthend=2&dayend=1

        addonNodejs.visualisation(yearstart, monthstart, daystart, yearend, monthend, dayend, limite, data, function (resultat, err, emptyData)
        {
            if (err !== null)
            {
                if (err === 1)
                {
                    res.status(404).send("beginning date");
                }
                else if (err === 2)
                {
                    res.status(404).send("ending date");
                }
                else
                {
                    res.status(404).send("beginning date > ending date");
                }
            }
            else if (emptyData === false)
            {
                res.writeHead(200, {
                  'Content-Type': 'application/json',
                  'Content-Length': resultat.length
                });
                res.end(resultat);
            }
            else
            {
                res.status(404).send("empty data");
            }
        });
    }
    else
    {
        res.status(404).send("invalid args");
    }
});

// Visualisation spécifique aux usernames et passwords, graphe en cercle et liste
app.get('/generator/visualisation2.json', function (req, res) {

    var yearstart = 0, monthstart = 0, daystart = 0, yearend = 0, monthend = 0, dayend = 0;
    var username = "", password = "";
    var data = -1;

    if (req.query.username !== undefined)
    {
        username = String(req.query.username);
    }

    if (req.query.password !== undefined)
    {
        password = String(req.query.password);
    }

    if (req.query.debut !== undefined)
    {
        var recuperation = "";
        var position = 0;

        //2019-12-04
        for (var i=0;i<req.query.debut.length;i++)
        {
            if (req.query.debut[i] !== '-') recuperation += req.query.debut[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            yearstart = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.debut.length;i++)
        {
            if (req.query.debut[i] !== '-') recuperation += req.query.debut[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            monthstart = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.debut.length;i++)
        {
            recuperation += req.query.debut[i];
        }

        if (isNaN(recuperation) === false)
        {
            daystart = parseInt(recuperation);
        }
    }

    if (req.query.fin !== undefined)
    {
        var recuperation = "";
        var position = 0;

        //2019-12-04
        for (var i=0;i<req.query.fin.length;i++)
        {
            if (req.query.fin[i] !== '-') recuperation += req.query.fin[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            yearend = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.fin.length;i++)
        {
            if (req.query.fin[i] !== '-') recuperation += req.query.fin[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            monthend = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.fin.length;i++)
        {
            recuperation += req.query.fin[i];
        }

        if (isNaN(recuperation) === false)
        {
            dayend = parseInt(recuperation);
        }
    }

    if (req.query.data !== undefined)
    {
        if (req.query.data === "username") data = 0;
        else if (req.query.data === "password") data = 1;
        else if (req.query.data === "usernamepassword") data = 2;
    }

    if (data !== -1)
    {
        addonNodejs.visualisation2(yearstart, monthstart, daystart, yearend, monthend, dayend, data, username, password, function (resultat, err, emptyData)
        {
            if (err !== null)
            {
                if (err === 1)
                {
                    res.status(404).send("beginning date");
                }
                else if (err === 2)
                {
                    res.status(404).send("ending date");
                }
                else if (err === 3)
                {
                    res.status(404).send("beginning date > ending date");
                }
                else if (err === 4)
                {
                    res.status(404).send("username not found");
                }
                else
                {
                    res.status(404).send("password not found");
                }
            }
            else if (emptyData === false)
            {
                res.writeHead(200, {
                  'Content-Type': 'application/json',
                  'Content-Length': resultat.length
                });
                res.end(resultat);
            }
            else
            {
                res.status(404).send("empty data");
            }
        });
    }
    else
    {
        res.status(404).send("invalid args");
    }
});

// Carte de la visualisation des usernames / passwords
app.get('/generator/carte2.csv', function (req, res) {

    var yearstart = 0, monthstart = 0, daystart = 0, yearend = 0, monthend = 0, dayend = 0;
    var username = "", password = "";
    var data = -1;

    if (req.query.username !== undefined)
    {
        username = String(req.query.username);
    }

    if (req.query.password !== undefined)
    {
        password = String(req.query.password);
    }

    if (req.query.debut !== undefined)
    {
        var recuperation = "";
        var position = 0;

        //2019-12-04
        for (var i=0;i<req.query.debut.length;i++)
        {
            if (req.query.debut[i] !== '-') recuperation += req.query.debut[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            yearstart = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.debut.length;i++)
        {
            if (req.query.debut[i] !== '-') recuperation += req.query.debut[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            monthstart = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.debut.length;i++)
        {
            recuperation += req.query.debut[i];
        }

        if (isNaN(recuperation) === false)
        {
            daystart = parseInt(recuperation);
        }
    }

    if (req.query.fin !== undefined)
    {
        var recuperation = "";
        var position = 0;

        //2019-12-04
        for (var i=0;i<req.query.fin.length;i++)
        {
            if (req.query.fin[i] !== '-') recuperation += req.query.fin[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            yearend = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.fin.length;i++)
        {
            if (req.query.fin[i] !== '-') recuperation += req.query.fin[i];
            else
            {
                position = i+1;
                break;
            }
        }

        if (isNaN(recuperation) === false)
        {
            monthend = parseInt(recuperation);
        }

        recuperation = "";

        for (var i=position;i<req.query.fin.length;i++)
        {
            recuperation += req.query.fin[i];
        }

        if (isNaN(recuperation) === false)
        {
            dayend = parseInt(recuperation);
        }
    }

    if (req.query.data !== undefined)
    {
        if (req.query.data === "username") data = 0;
        else if (req.query.data === "password") data = 1;
        else if (req.query.data === "usernamepassword") data = 2;
    }

    if (data !== -1)
    {
        addonNodejs.visualisation2Carte(yearstart, monthstart, daystart, yearend, monthend, dayend, data, username, password, function (resultat, err, emptyData)
        {
            if (err !== null)
            {
                if (err === 1)
                {
                    res.status(404).send("beginning date");
                }
                else if (err === 2)
                {
                    res.status(404).send("ending date");
                }
                else if (err === 3)
                {
                    res.status(404).send("beginning date > ending date");
                }
                else if (err === 4)
                {
                    res.status(404).send("username not found");
                }
                else
                {
                    res.status(404).send("password not found");
                }
            }
            else if (emptyData === false)
            {
                res.writeHead(200, {
                  'Content-Type': 'text/csv',
                  'Content-Length': resultat.length
                });
                res.end(resultat);
            }
            else
            {
                res.status(404).send("empty data");
            }
        });
    }
    else
    {
        res.status(404).send("invalid args");
    }
});

// Redirection de l'utilisateur vers le fichier index.html
app.get('/', function (req, res) {
    res.redirect('/static/html/index.html');
});

// Redirection de l'utilisateur vers le fichier index.html
app.get('/index.html', function (req, res) {
    res.redirect('/static/html/index.html');
});

// Erreur 404 si l'utilisateur a entré une URI incorrecte
app.all('*', function (req, res) {
    res.status(404).send('404');
});


http.createServer(app).listen(8888, 'localhost')
