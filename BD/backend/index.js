const express = require('express');
const path = require('path');
const cors = require('cors');
const executeQuery = require('./api/dbQuery');
const queries = require('./api/sql_queries');
const utils = require('./api/utils');
const formidable = require('formidable');
const data = require('./api/constants');
const fs = require('fs');


const app = express();


app.use(express.static(path.join(__dirname, 'client/build')));
app.use(cors());


app.get('/api/getUsers', (req,res) => {
    executeQuery(queries.selectAllUsersSQL, []).then(({rows}) => {
        const users = rows.map(utils.tableToUser);
        res.json(users);
    });
});


app.get('/api/getUser/:username', (req,res) => {
    executeQuery(queries.selectUserSQL, [req.params.username]).then(({rows}) => {
        if (rows.length > 0) {
            const user = utils.tableToUser(rows[0]);
            res.json(
                {
                    found: true,
                    user: user
                }
            );
        } else {
            res.json({found: false});
        }
    });
});

app.get('/api/getUserByID/:ID', (req,res) => {
    executeQuery(queries.selectUserByIdSQL, [req.params.id]).then(({rows}) => {
        if (rows.length > 0) {
            const user = utils.tableToUser(rows[0]);
            res.json(
                {
                    found: true,
                    user: user
                }
            );
        } else {
            res.json({found: false});
        }
    });
});

app.get('/api/getStatus/:id', (req,res) => {
    executeQuery(queries.selectStatusSQL, [req.params.id]).then(({rows}) => {
        if (rows.length > 0) {
            const status = utils.viewToFullStatus(rows[0]);
            res.json(
                {
                    found: true,
                    status: status
                }
            );
        } else {
            res.json({found: false});
        }
    });
});

app.get('/api/getStatuses', (req,res) => {
    executeQuery(queries.selectAllStatusesSQL, []).then(({rows}) => {
        const statuses = rows.map(utils.viewToFullStatus);
        res.json(statuses);
    });
});

app.get('/api/getUserStatuses/:userID', (req,res) => {
    executeQuery(queries.selectUserStatusesSQL, [req.params.userID]).then(({rows}) => {
        const statuses = rows.map(utils.tableToStatus);

        res.json(statuses);
    });
});

app.get('/api/getReplies/:statusID', (req,res) => {
    executeQuery(queries.selectRepliesSQL, [req.params.statusID]).then(({rows}) => {
        const statuses = rows.map(utils.viewToFullStatus);
        res.json(statuses);
    });
});

app.get('/api/getStats', (req,res) => {
    executeQuery(queries.selectStatsSQL, []).then(({rows}) => {
        const stats = utils.viewToStats(rows[0]);

        console.log(stats.oldestAccount);
        res.json(stats);
    });
    console.log('Sent list of statuses');
});

app.post('/api/newFile', (req, res) => {
    new formidable.IncomingForm()
    .parse(req)
    .on('file', (name, file) => {
       fs.readFile(file.path, (err, data) => {
           utils.processFile(data.toString());
       });
      });
});

const port = process.env.PORT || 5000;
app.listen(port);

console.log('App is listening on port ' + port);