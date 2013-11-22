create table config (
	ord integer primary key asc,
	name, val
);

insert into config (name, val) values ("aaa", "bbb"), (123, 456);
