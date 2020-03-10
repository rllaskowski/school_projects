import React , {useState, useEffect} from "react";
import Card from '@material-ui/core/Card'
import {makeStyles} from "@material-ui/core/styles";
import {CardContent} from "@material-ui/core";
import Typography from "@material-ui/core/Typography";
import CardHeader from "@material-ui/core/CardHeader";
import Avatar from "@material-ui/core/Avatar";
import Divider from "@material-ui/core/Divider";
import Grid from '@material-ui/core/Grid';
import ThumbUpIcon from '@material-ui/icons/ThumbUp';
import Box from "@material-ui/core/Box";
import {getStatus, getAllStatuses, getStatusByID, getRepliesTo} from '../api/api'
import List from '@material-ui/core/List';
import ListItem from "@material-ui/core/ListItem";
import Link from '@material-ui/core/Link';
import VerifiedUserIcon from '@material-ui/icons/VerifiedUser';

const useStyles = makeStyles({
    statusContainer: {
        itemAlign: 'center'
    },
    statusCard: {
    },
    statusInfo: {
        marginTop: '10px'
    },
    screenName: {
        fontSize: 15
    },
    username: {
        fontSize: 13
    },
    statusContent: {
        padding: '20px'
    },
    statusText: {
        marginBottom: '5px',
    }
});

Date.prototype.stringify = function() {
    var mm = this.getMonth() + 1; // getMonth() is zero-based
    var dd = this.getDate();

    return [this.getFullYear(),
        (mm>9 ? '' : '0') + mm,
        (dd>9 ? '' : '0') + dd
    ].join('-');
};

function StatusCard({status, user}) {
    const classes = useStyles();

    return (
        <Card className={classes.statusCard} >
            <CardHeader
                title={
                    <Box fontWeight={'bold'}>
                        <Link href={`/user/${user?.username}`} color={'inherit'}>
                     
                            {user?.screenName}
                            
                        </Link>
                        {
                            user?.verified? <VerifiedUserIcon/> : <React.Fragment/>   
                        }
                    </Box>
                }
                avatar={
                    <Link href={`/user/${user?.username}`}>
                        <Avatar src={user?.imgSrc} className={classes.avatar}/>
                    </Link>
                }
                subheader={
                    <React.Fragment>
                    {
                        `@${user?.username} w dniu ${new Date(Date.parse(status?.createdAt)).stringify()}`
                    }
                    </React.Fragment>
                }
            />
            <Divider/>
            <CardContent >
                <Link href={`/status/${status?.id}`}underline={'none'} color={'inherit'}>
                    <Box className={classes.statusText} className={classes.statusContent}>
                        <Typography>
                            {status?.description}
                        </Typography>
                    </Box>
                </Link>
                <Divider/>
                <Grid container alignItems={'right'} spacing={2} className={classes.statusInfo} >
                    <Grid item>
                        <ThumbUpIcon/> {status?.thumbsUp}
                    </Grid>
                </Grid>
            </CardContent>
        </Card>
    );
}

function Status({match}) {
    const [data, setData] = useState();
    const [replies, setReplies] = useState([]);

    const classes = useStyles();

    useEffect(() => {
        getStatusByID(match.params.statusID).then(setData);
        getRepliesTo(match.params.statusID).then(setReplies);
    }, [match.params.statusID]);


    return (
        <React.Fragment>
            {
            data? 
            <React.Fragment>
                <StatusCard status={data?.status} user={data?.user}/>

                {
                    replies.length > 0?
                    <React.Fragment>
                        <Box textAlign={'center'} style={{marginBottom: '10px', marginTop: '50px'}}>
                            <Typography component={'h5'} variant={'h4'} justify={'center'}>
                                Odpowiedzi
                            </Typography>
                        </Box>
                        <StatusList list={replies}/>
                    </React.Fragment>
                    :
                    <Box textAlign={'center'} style={{marginBottom: '50px', marginTop: '50px'}}>
                        <Typography component={'h5'} variant={'h4'} justify={'center'}>
                            Nie ma odpowiedzi do tego postu
                        </Typography>
                    </Box>
                }   
            </React.Fragment>
            :
            <Box textAlign={'center'} style={{marginBottom: '50px', marginTop: '20px'}}>
                <Typography component={'h4'} variant={'h3'} justify={'center'}>
                    Nie znaleziono postu
                </Typography>
            </Box>
            }
        </React.Fragment>
    );

}

function StatusList({list, user}) {
    return (
        <React.Fragment>
            <List >
                {
                    list.map((el, itx) => {
                        if (user) {
                            return (
                                <ListItem key={itx} disableGutters={true}>
                                    <StatusCard status={el} user={user} />
                                </ListItem>
                            );
                        } else {
                            return (
                                <ListItem key={itx} disableGutters={true}>
                                    <StatusCard status={el.status} user={el.user} />
                                </ListItem>
                            );
                        }
                    })
                }
            </List>
        </React.Fragment>
    );
}


function AllStatus(props) {
    const [statusList, setStatusList] = useState([]);

    useEffect(() => {
        getAllStatuses().then(setStatusList);

    }, []);

    return (
        <React.Fragment>
            <Box textAlign={'center'} style={{marginBottom: '20px', marginTop: '20px'}}>
                <Typography component={'h3'} variant={'h3'} justify={'center'}>
                    Posty
                </Typography>
            </Box>
            <Divider/>
            <StatusList list={statusList}/>
        </React.Fragment>
    );
}

export default Status;
export {
    StatusList,
    AllStatus
};