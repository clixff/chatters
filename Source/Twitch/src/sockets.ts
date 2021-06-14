import { Server, Socket } from 'socket.io';
import http from 'http';
import { getTwitchAppClientID, getTwitchAuthData } from './misc';
import fetch from 'node-fetch';
import { chatClient } from '.';

export default class SocketsServer
{
    constructor(RawServer: http.Server)
    {
        this.io = new Server(RawServer);
        console.log('[SocketsServer] Sockets server created');

        this.onConnection = this.onConnection.bind(this);
        this.onDisconnect = this.onDisconnect.bind(this);
        this.onTwitchTokenLoaded = this.onTwitchTokenLoaded.bind(this);
        this.onGameLevelLoaded = this.onGameLevelLoaded.bind(this);

        this.io.on('connection', this.onConnection);
    }

    onDisconnect(reason: string): void
    {
        this.socket = null;
        console.log(`[SocketsServer] Client disconnected. Reason: ${reason}`);

        if (chatClient)
        {
            chatClient.disconnect();
        }
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

        this.socket.on('level-loaded', this.onGameLevelLoaded);
    }

    async onTwitchTokenLoaded(twitchToken: string): Promise<void>
    {
        try
        {
            console.log(`[SocketsServer] Twitch token loaded: "${twitchToken}"`);
        
            if (!twitchToken.length)
            {
                this.sendTwitchAuthData(false, '');

                if (chatClient)
                {
                    chatClient.disconnect();
                }
            }
            else
            {
                const authData = await getTwitchAuthData(twitchToken);
                
                if (authData)
                {
                    this.sendTwitchAuthData(authData.bSignedIn, authData.name);

                    if (chatClient)
                    {
                        chatClient.listen(authData.name.toLowerCase(), twitchToken, authData.id);
                    }
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

        if (chatClient)
        {
            chatClient.disconnect();
        }
    }

    onViewerJoin(viewerName: string): void
    {
        if (this.socket)
        {
            this.socket.emit('viewer-join', viewerName);
        }
    }

    onViewerMessage(viewerName: string, message: string): void
    {
        if (this.socket)
        {
            this.socket.emit('viewer-message', viewerName.toLowerCase(), message);
        }
    }

    onGameLevelLoaded(): void
    {
        const bSpawnDebugFakeViewers = false;

        if (bSpawnDebugFakeViewers)
        {
            function generateString(): string
            {
                const originalString = (Math.round(Math.random() * Number.MAX_SAFE_INTEGER).toString(16)).toUpperCase().repeat(2);

                const minLength = 10;
                const maxLength = 25;

                const length = minLength + Math.round(Math.random() * (maxLength - minLength));

                return originalString.substr(0, length);
            }

            const maxBots = 75;

            const join = (i: number): void =>
            {
                if (i == maxBots)
                {
                    return;
                }

                this.onViewerJoin(generateString());

                const timeout = Math.random() * 500;

                setTimeout(() =>
                {
                    join(i + 1);
                }, timeout);                
            };

            join(0);
        }
    }

    io?: Server | null;

    /** Singleton client socket instance */
    socket?: Socket | null;
}