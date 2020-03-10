import React from 'react';
import Box from "@material-ui/core/Box";
import Typography from "@material-ui/core/Typography";
import FormControl from "@material-ui/core/FormControl";
import Input from "@material-ui/core/Input";
import Divider from "@material-ui/core/Divider";
import Button from '@material-ui/core/Button';

const NewData = () => {
    let file = undefined;

    const handleSubmit = () => {
        let file = document.getElementById('newFile');
        let form = new FormData();
        form.append('newData', file.files[0]);
        const http = new XMLHttpRequest();
        http.open("POST", "http://localhost:5000/api/newFile", true);
        http.send(form);
    };

    return (
        <React.Fragment>
            <Box textAlign={'center'}  style={{marginBottom: '20px', marginTop: '20px'}}>
                <Typography component={'h3'} variant={'h3'} justify={'center'}>
                    Wgraj nowe dane
                </Typography>
            </Box>
            <Divider/>

            <Box style={{marginTop: '50px'}}>
                <FormControl>
                    <Input type="file" name="file" id="newFile"/>
                    <Button onClick={handleSubmit}>Wyślij</Button>
                </FormControl>
            </Box>
        </React.Fragment>
    );
};

export default NewData;