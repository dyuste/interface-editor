ENTITY Inv IS
	PORT( I : IN BIT; O : OUT BIT);
END Inv;

ARCHITECTURE functional OF Inv IS
	BEGIN
		O <= NOT I;

END functional;
