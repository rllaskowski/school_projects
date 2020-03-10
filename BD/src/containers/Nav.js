import React from 'react';
import List from '@material-ui/core/List'
import ListItem from "@material-ui/core/ListItem";
import ListSubheader from "@material-ui/core/ListSubheader";
import {makeStyles} from "@material-ui/core/styles";
import SearchIcon from '@material-ui/icons/Search';
import Paper from "@material-ui/core/Paper";
import InputBase from "@material-ui/core/InputBase";
import IconButton from "@material-ui/core/IconButton";
import Link from '@material-ui/core/Link';
import ListItemIcon from "@material-ui/core/ListItemIcon";
import EqualizerIcon from '@material-ui/icons/Equalizer';
import ListItemText from "@material-ui/core/ListItemText";
import ViewListIcon from '@material-ui/icons/ViewList';
import SupervisorAccountIcon from '@material-ui/icons/SupervisorAccount';
import Routes from "../constants/routes";
import Grid from "@material-ui/core/Grid";
import Divider from "@material-ui/core/Divider";
import BorderColorIcon from '@material-ui/icons/BorderColor';
import Typography from "@material-ui/core/Typography";
import Box from "@material-ui/core/Box";
const useStyles = makeStyles({
    root: {
        width: '100%'
    },
    listElement: {
        padding: '20px',
        backgroundColor: '#35baf6',
        borderRadius: '30px',
        marginTop: '20px'
    },
    linkText: {
        color: 'white',
        fontDisplay: 'bold'
    }
});


function SearchUser() {
    const classes = useStyles();
    let user = '';

    const handleChange = event => {
        user = event.target.value;
        console.log(user);
    };

    return (
        <Paper style={{width: '100%'}}>
            <InputBase
                style={{paddingLeft: '15px', width: '80%'}}
                className={classes.input}
                placeholder="Znajdź użytkownika"
                inputProps={{'aria-label': 'search-user'}}
                onChange={handleChange}
            />
            <Link onClick={()=>{window.location=`/user/${user}`;}} component={'a'}>
                <IconButton >
                    <SearchIcon/>
                </IconButton>

            </Link>
        </Paper>
    );
}

const NavigationLink = (props) => {
    const classes = useStyles();
    return (
        <Link href={props.href} component={'a'} underline={'none'}>
            <ListItem disableGutters={true} className={classes.listElement}>
                <ListItemIcon>
                    {props.icon}
                </ListItemIcon>
                <ListItemText primary={<Box fontWeight={'bold'}>{props.name}</Box>} className={classes.linkText}/>
            </ListItem>
        </Link>
    );
};

const Navigation = (props) => {
    return (
        <List >
            <ListItem disableGutters={true}>
                <SearchUser/>
            </ListItem>
            <NavigationLink icon={<EqualizerIcon />} href={Routes.STATS} name={"Statystyki"}/>
            <NavigationLink icon={<SupervisorAccountIcon />} href={Routes.USERS} name={"Wszyscy użytkownicy"}/>
            <NavigationLink icon={<ViewListIcon />} href={Routes.STATUSES} name={"Wszystkie posty"}/>
            <NavigationLink icon={<BorderColorIcon />} href={Routes.NEW_DATA} name={"Wgraj nowe dane"}/>
        </List>
    );
};

export default Navigation;