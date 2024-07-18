create table if not exists utilizatori(
    ID            integer       primary key    not null,
    "password"    varchar(20)                  not null,
    "name"        varchar(20)                  not null,
    logged_in     integer                              ,
    card1         integer                              ,
    card2         integer                              ,
    savings       integer                              ,
    NRdepuneri    integer                              ,
    NRretrageri   integer         
);

insert into utilizatori (id, "password", "name", logged_in, card1, card2, savings, NRdepuneri, NRretrageri) values
(111, 'pisica1', 'Andrei', 0, 250, 300, 500, 3, 0);

insert into utilizatori (id, "password", "name", logged_in, card1, card2, savings, NRdepuneri, NRretrageri) values
(222, 'pisica2', 'Antonia', 0, 4000, 50, 200, 3, 1);

insert into utilizatori (id, "password", "name", logged_in, card1, card2, savings, NRdepuneri, NRretrageri) values
(333, 'pisica3', 'Maria', 0, 2000, 200, 200, 5, 2);

insert into utilizatori (id, "password", "name", logged_in, card1, card2, savings, NRdepuneri, NRretrageri) values
(444, 'pisica4', 'Darius', 0, 1000, 100, 30, 3, 1);