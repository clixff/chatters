import tmi from 'tmi.js';
import { socketsServer } from '.';
import { GetTwitchValidDisplayName } from './misc';

export default class ChatClient
{
    constructor()
    {
        this.onConnect = this.onConnect.bind(this);
        this.onMessage = this.onMessage.bind(this);
    }

    listen(username: string, token: string, userId: number): void
    {
        try
        {
            if (this.client)
            {
                this.disconnect();
            }

            console.log(`[ChatClient] Listening`);

            this.params.identity = {
                username: username,
                password: token
            };
            this.params.channels = [ username ];
    
            this.client = new tmi.client(this.params);
    
            this.client.connect();
    
            this.client.on('connected', this.onConnect);
    
            this.client.on('message', this.onMessage);
    
            this.channelID = `${userId}`;
        }
        catch (error)
        {
            console.error(error);
        }
    }

    onConnect(addr: string, port: number): void
    {
        console.log(`[ChatClient] Connected to ${addr}:${port}`);
    }

    onMessage(channel: string, context: tmi.ChatUserstate, message: string): void
    {
        message = message.trim();

        if (!context)
        {
            return;
        }

        if (context['message-type'] == 'chat' && context['room-id'] == this.channelID && context['user-id'])
        {
            const messageLowerCased = message.toLowerCase();

            const userName = GetTwitchValidDisplayName(context.username || '', context['display-name'] || '');

            if (!userName)
            {
                return;
            }

            if (messageLowerCased == '!play' || messageLowerCased.startsWith('!play '))
            {
                if (socketsServer)
                {
                    socketsServer.onViewerJoin(userName);
                }
            }
            else if (message.length)
            {
                if (socketsServer)
                {
                    socketsServer.onViewerMessage(userName, message);
                }
            }
        }
    }

    disconnect(): void
    {
        if (this.client)
        {
            console.log(`[ChatClient] Disconnecting`);
            this.client.disconnect();
            this.client = null;
        }
    }

    client: tmi.Client | null = null;
    params: tmi.Options = {
        connection:
        {
            reconnect: true,
            secure: true
        },
        options:
        {
            debug: false
        }
    };

    channelID = ''; 
}   