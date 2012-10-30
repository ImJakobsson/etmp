const COLOR_RED = 0xFF0000FF;
const COLOR_GREEN = 0x33AA33FF;
const COLOR_ORANGE = 0xFF8000FF;
const COLOR_BLUE = 0x0080FFFF;

function onPlayerConnect(playerid)
{
	sendMessageToAll(0xFF0000FF, "Gracz " + getPlayerName(playerid) + " do��czy� do serwera!");
	return 1;
}
addEventHandler("onPlayerConnect", onPlayerConnect);

function onPlayerDisconnect(playerid)
{
	sendMessageToAll(0xFF0000FF, "Gracz " + getPlayerName(playerid) + " opu�ci� serwer!");
	return 1;
}
addEventHandler("onPlayerDisconnect", onPlayerDisconnect);

function onPlayerCommand(playerid, command)
{
	local cmd = split(command, " ");
	
	switch(cmd[0])
	{
		case "/me":		
		{
			if(cmd.len() < 2)
			{
				sendPlayerMessage(playerid, 0xC0C0C0FF, "Wpisz: /me <akcja>");
				return 1;
			}
			
			local tresc = command.slice(cmd[0].len()+1);
			sendPlayerMessage(playerid, 0xC278FFFF, "* " + getPlayerName(playerid) + " " + tresc);
			break;
		}
		case "/help":
		{
			sendPlayerMessage(playerid, 0x00FF00FF, "Dost�pne komendy:");
			sendPlayerMessage(playerid, 0x33AA33FF, "/me <akcja> - odgrywasz akcj� rp.");
			sendPlayerMessage(playerid, 0x33AA33FF, "/help - Wy�wietla t� liste.");
			sendPlayerMessage(playerid, 0x33AA33FF, "/pm lub /w <id> <tekst> - Wysy�asz prywatn� wiadomo��.");
			break;
		}
		case "/pm":
		case "/w":
		{
			if(cmd.len() < 3)
			{
				sendPlayerMessage(playerid, 0xC0C0C0FF, "Wpisz: (/pm lub /w) <id> <tekst>");
				return 1;
			}
			
			local pid = cmd[1].tointeger();
			
			if(!isPlayerConnected(pid))
			{
				sendPlayerMessage(playerid, 0xC0C0C0FF, "Gracz o podanym id jest off-line.");
				return 1;
			}
			
			if(pid == playerid)
			{
				sendPlayerMessage(playerid, 0xC0C0C0FF, "Nie mo�esz wys�a� wiadomo�ci do siebie.");
				return 1;			
			}
			
			local tresc = command.slice(cmd[0].len()+cmd[1].len()+2);
			
			sendPlayerMessage(playerid, 0xFFBB00FF, "(( Wiadomo�� do - " + getPlayerName(pid) + ": " + tresc + " ))");
			sendPlayerMessage(pid 0xFF9100FF, "(( Wiadomo�� od - " + getPlayerName(playerid) + ": " + tresc + " ))");			
			break;
		}
		case "/ping":
		{
			local players = 0;
			for ( local i = 0; i < MAX_PLAYERS; i ++ )
			{
				if(isPlayerConnected(i))
				{
					sendPlayerMessage(playerid, 0xFFFFFFFF, getPlayerName(i) + " " + getPlayerPing(i));
					players++;
				}
			}
			
			if(!players)
			{
				sendPlayerMessage(playerid, 0xFFFFFFFF, "Brak graczy dla kt�rych mo�na wy�wietli� ping.");
			}
			break;
		}
		default:
		{
			sendPlayerMessage(playerid, 0xFF0000FF, "* Brak takiej komendy!");
			break;
		}		
	}
	return 1;
}
addEventHandler("onPlayerCommand", onPlayerCommand);