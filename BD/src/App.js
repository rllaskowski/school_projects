import React from 'react';
import {BrowserRouter as Router, Route} from 'react-router-dom';
import Routes from "./constants/routes";
import Grid from '@material-ui/core/Grid';
import Status, {AllStatus} from './containers/Status';
import User from './containers/User';
import Navigation from "./containers/Nav";
import Stats from "./containers/Stats";
import {UserList} from './containers/User';
import Box from '@material-ui/core/Box';
import NewData from './containers/NewData';
import Paper from "@material-ui/core/Paper";

function App() {
  return (
      <Grid container justify={'center'} >
          <Grid item xs={8}>
      <Paper elevation={3}>
          <Grid container justify={'center'} spacing={2}  bgcolor={'info.main'} style={{minHeight:'100vh'}}>
              <Grid item xs={4} >
                  <Navigation/>
              </Grid>
              <Grid item xs={7}>
                  <Router>
                      <Route exact path={Routes.STATS} component={Stats}/>
                      <Route exact path={Routes.STATUS} component={Status}/>
                      <Route exact path={Routes.USER} component={User}/>
                      <Route exact path={Routes.USERS} component={UserList}/>
                      <Route exact path={Routes.STATUSES} component={AllStatus}></Route>
                      <Route exact path={Routes.NEW_DATA} component={NewData}></Route>
                  </Router>
              </Grid>
          </Grid>
      </Paper>
          </Grid>
      </Grid>
  );
}

export default App;
