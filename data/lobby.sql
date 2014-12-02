CREATE TABLE `room` (
    `id` INTEGER PRIMARY KEY AUTOINCREMENT,
    `hostaddress` varchar(21),
    `playernum` int,
    `name` varchar(50),
    `gamemode` varchar(20),
    `banpackages` text,
    `operationtimeout` int,
    `nullificationcountdown` int,
    `randomseat` boolean,
    `enablecheat` boolean,
    `freechoose` boolean,
    `forbidaddingrobot` boolean,
    `disablechat` boolean,
    `firstshowingreward` boolean,
    `requirepassword` boolean
);
