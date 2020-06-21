var token = require('./token');

const discord = require('discord.js');
const SerialPort = require('serialport');
const bot = new discord.Client();

const statusChannelId = '632269821513302037';
const recordChannelId = '724239637467627541';

const serial = new SerialPort('COM36', {baudRate: 9600});

const fs = require('fs');

var rxbuffer=  '';

serial.on('error', e => {
    console.log(e);
})

serial.on('data', data => {
    rxbuffer += data.toString('ascii');
    if(rxbuffer[rxbuffer.length - 1] == '\n') {
        console.log(rxbuffer);

        let args = rxbuffer.trim().split(' ');

        console.log(args);

        let strikes = args[1];
        let responseTime = args[2];

        
        let hsobj = fs.readFileSync('./highscore.json', {encoding: 'utf-8'});
        let highscore = JSON.parse(hsobj);
        let ranked = false;
        
        if(highscore.strikes < strikes) ranked = true;

        
        let msg = `${time.toLocaleString()}\n${args[1]}회\n반응시간${args[2]}ms\n`;
                    
        if(ranked == true) msg = ':star::star::star::star::star::star::star::star::star::star:' 
                                + msg
                                + ':star::star::star::star::star::star::star::star::star::star:';
        else msg = '********************' + msg + '********************';
        
        let time = new Date();
        bot.channels.cache.find(ch => ch.id == recordChannelId).send(msg);

        rxbuffer = '';
        setTimeout(() => {
            serial.write(Buffer.alloc(3, 'OK\n', 'ascii'));
        }, 1000);
    }
})

bot.on('ready', () => {
	console.log(`Logged in as ${bot.user.tag}!`);
	let t = new Date();
	bot.channels.cache.find(ch => ch.id == statusChannelId)
        .send(`Start Service | ${bot.user.tag} | ${t.toLocaleString()}`);
    
});

bot.on('error', e => {
    console.log(e);
})

process.on('uncaughtException', e => {
	console.log(e);
})

bot.login(token);