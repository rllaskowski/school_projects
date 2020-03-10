import React, {useState, useEffect} from 'react';
import {CardHeader, Container} from "@material-ui/core";
import Card from "@material-ui/core/Card";
import {StatusList} from "./Status";
import {getAllStatuses, getAllUsers, getStatus, getUser, getUserStatuses} from '../api/api';
import List from '@material-ui/core/List';
import ListItem from '@material-ui/core/ListItem'
import {makeStyles} from "@material-ui/core/styles";
import Avatar from "@material-ui/core/Avatar";
import CardContent from "@material-ui/core/CardContent";
import ListItemText from "@material-ui/core/ListItemText";
import ListItemAvatar from "@material-ui/core/ListItemAvatar";
import Typography from "@material-ui/core/Typography";
import Divider from "@material-ui/core/Divider";
import Link from '@material-ui/core/Link';
import Box from '@material-ui/core/Box';
import VerifiedUserIcon from '@material-ui/icons/VerifiedUser';
import CardMedia from '@material-ui/core/CardMedia';

const useStyles = makeStyles({
    userCard: {

    },
    root: {
        width: '100%',
        maxWidth: 360,
    },
    inline: {
        display: 'inline',
    },
});

Date.prototype.stringify = function() {
    var mm = this.getMonth() + 1; // getMonth() is zero-based
    var dd = this.getDate();

    return [this.getFullYear(),
        (mm>9 ? '' : '0') + mm,
        (dd>9 ? '' : '0') + dd
    ].join('-');
};

function UserCard(user) {
    const classes = useStyles();

    return (
        <Card className={classes.userCard}>
             <CardMedia
                style={{paddingTop:'100px', backgroundColor: '#35baf6'}}
                image={user.backgroundSrc}
                title="Paella dish"
                
            />
            <CardHeader
                title={
                    <Box fontWeight={'bold'}>
                        {
                            user?.screenName
                        }
                        {    
                            user?.verified? <VerifiedUserIcon/> : <React.Fragment/>   
                        }
                    </Box>
                }
                subheader={
                    <React.Fragment>
                    {
                        `@${user?.username} - konto stworzone w dniu ${new Date(Date.parse(user?.createdAt)).stringify()}`
                    }
                    </React.Fragment>
                }
                avatar={
                    <Avatar alt={user.screenName} src={user?.imgSrc} className={classes.avatar}/>
                }
            />
            <Divider/>
           
            <CardContent style={{minHeight:'100px'}}>
                <Box textAlign={'center'}>
                <Typography style={{fontWeight : 'bold'}}>
                    Statystyki użytkownika
                </Typography>
                <Typography >
                    Ilość postów: {user.statuses}
                </Typography>
                <Typography >
                    Ilość followersów: {user.followersCount}
                </Typography>
                <Typography >
                    Konto zweryfikowane: {user.verified? 'Tak' : 'Nie'}
                </Typography>
                <Typography >
                    Polubione posty: {user.favouriteCount}
                </Typography>
                </Box>
            </CardContent>
        </Card>
    );
}

function UserDescription(user) {
    const classes = useStyles();

    return (
        <Link component={'a'} href={`/user/${user.username}`} color={'inherit'}>
            <ListItem alignItems="flex-start" style={{padding: '20px'}}>
                <ListItemAvatar>
                    <Avatar alt={user.screenName} src={user.imgSrc} />
                </ListItemAvatar>
                <ListItemText
                    primary={
                        <React.Fragment>
                        {
                            user?.screenName
                        }
                        {    
                            user?.verified? <VerifiedUserIcon/> : <React.Fragment/>   
                        }
                        </React.Fragment>
                    }
                    secondary={
                        <React.Fragment >
                            
                            <Typography
                                component="span"
                                variant="body2"
                                className={classes.inline}
                                color="textPrimary"
                            >
                                {`@${user?.username} konto stworzone w dniu ${new Date(Date.parse(user?.createdAt)).stringify()}`}
                            </Typography>

                        </React.Fragment>
                    }
                />
            </ListItem>
        </Link>
    );
}

function User({match}) {
    const [user, setUser] = useState(undefined);
    const [statusList, setStatusList] = useState([]);

    useEffect(() => {
        getUser(match.params.username).then(user=>{
            if (user) {
                setUser(user);
                getUserStatuses(user.id).then(setStatusList)}
                }   
            );
        
    }, [match.params.username]);

    return (
        <React.Fragment>
            {
                user ? 
                <React.Fragment>
                    <UserCard {...user} statuses={statusList.length}/> 
                    <StatusList list={statusList} user={user}/>
                </React.Fragment>
                : 
                <Box textAlign={'center'} style={{marginBottom: '50px', marginTop: '20px'}}>
                <Typography component={'h4'} variant={'h3'} justify={'center'}>
                    Nie znaleziono użytkownika
                </Typography>
                </Box>
            }
            
            
        </React.Fragment>
    );
}

function UserList(props) {
    const [userList, setUserList] = useState([]);

    useEffect(() => {
        getAllUsers().then(setUserList);
    }, []);

    const classes = useStyles();

    return (
        <React.Fragment>
            <Box textAlign={'center'} style={{marginBottom: '20px', marginTop: '20px'}}>
                <Typography component={'h3'} variant={'h3'} justify={'center'}>
                    Użytkownicy
                </Typography>
            </Box>
            <Divider/>
            <List>
                {
                    userList.map((user) => {
                        return (
                            <React.Fragment>
                               <UserDescription {...user}/>
                            <Divider variant="inset" component="li" />
                            </React.Fragment>
                        );
                    })
                }
            </List>
        </React.Fragment>
    )
}

export default User;
export {
    UserList
};