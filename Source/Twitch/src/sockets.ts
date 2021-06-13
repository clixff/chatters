import { Server, Socket } from 'socket.io';
import http from 'http';
import { getTwitchAppClientID, getTwitchAuthData } from './misc';
import fetch from 'node-fetch';

export default class SocketsServer
{
    constructor(RawServer: http.Server)
    {
        this.io = new Server(RawServer);
        console.log('[SocketsServer] Sockets server created');

        this.onConnection = this.onConnection.bind(this);
        this.onDisconnect = this.onDisconnect.bind(this);
        this.onTwitchTokenLoaded = this.onTwitchTokenLoaded.bind(this);


        this.io.on('connection', this.onConnection);
    }

    onDisconnect(reason: string): void
    {
        this.socket = null;
        console.log(`[SocketsServer] Client disconnected. Reason: ${reason}`);
    }

    onConnection(socket: Socket): void
    {
        if (this.socket)
        {
            console.log('[SocketsServer] Error: Client already connected');
            return;
        }
        
        this.socket = socket;
        console.log('[SocketsServer] Client connected.');

        this.socket.on('disconnect', this.onDisconnect);

        this.socket.on('twitch-token-loaded', this.onTwitchTokenLoaded);
        
        this.socket.on('twitch-logout', this.onTwitchLogout);
    }

    async onTwitchTokenLoaded(twitchToken: string): Promise<void>
    {
        try
        {
            console.log(`[SocketsServer] Twitch token loaded: "${twitchToken}"`);
        
            if (!twitchToken.length)
            {
                this.sendTwitchAuthData(false, '');
            }
            else
            {
                const authData = await getTwitchAuthData(twitchToken);
                
                if (authData)
                {
                    this.sendTwitchAuthData(authData.bSignedIn, authData.name);
                }
            }
        }
        catch (error)
        {
            console.error(error);
        }
    }

    sendTwitchAuthData(bSignedIn: boolean, displayName: string): void
    {
        if (this.socket)
        {
            this.socket.emit('twitch-auth-data-loaded', bSignedIn, displayName);
        }
    }

    sendNewTwitchToken(token: string): void
    {
        if (this.socket)
        {
            this.socket.emit('twitch-token-updated', token);

            this.onTwitchTokenLoaded(token);
        }
    }

    onTwitchLogout(oldToken: string): void
    {
        console.log('[SocketsServer] Twitch logout. Revoking old token..');

		const revokeURL = `https://id.twitch.tv/oauth2/revoke?client_id=${getTwitchAppClientID()}&token=${oldToken}`;
        fetch(revokeURL, {
            method: 'POST'
        });
    }

    io?: Server | null;

    /** Singleton client socket instance */
    socket?: Socket | null;
}