import React, {useEffect, useState} from "react";
import Typography from "@material-ui/core/Typography";
import Box from "@material-ui/core/Box";
import List from '@material-ui/core/List';
import ListItem from '@material-ui/core/ListItem'
import {getStats} from '../api/api';
import Divider from '@material-ui/core/Divider';

Date.prototype.stringify = function() {
    var mm = this.getMonth() + 1; // getMonth() is zero-based
    var dd = this.getDate();

    return [this.getFullYear(),
        (mm>9 ? '' : '0') + mm,
        (dd>9 ? '' : '0') + dd
    ].join('-');
};


function Statistic({value}) {
    return (
        <ListItem key={value}>
            <Typography variant={'h4'}>{value}</Typography>
        </ListItem>
    );
}

function Stats(props) {

    const [stats, setStats] = useState({});

    useEffect(()=> {
        getStats().then(setStats);
    }, []);
    return (
        <React.Fragment>
            <Box textAlign={'center'} style={{marginBottom: '20px', marginTop: '20px'}}>
                <Typography component={'h3'} variant={'h3'} justify={'center'}>
                    Statystyki
                </Typography>
            </Box>
            <Divider/>
            <List>
                <Statistic value={`Ilość profili: ${stats?.profiles}`}/>
                <Statistic value={`Ilość postów: ${stats?.statuses}`}/>
                <Statistic value={`Najdłuższy post: ${stats?.longestStatus} znaków`}/>
                <Statistic value={`Największa ilość polubień: ${stats?.maxLikes}`}/>
                <Statistic value={`Największa ilość followersów: ${stats?.maxFollowers}`}/>
                <Statistic value={`Najstarsze konto: ${new Date(Date.parse(stats?.oldestAccount)).stringify()}`}/>
                <Statistic value={`Zweryfikowani użytkownicy: ${stats?.verified}`}/>
            </List>


        </React.Fragment>
    );
}
export default Stats;