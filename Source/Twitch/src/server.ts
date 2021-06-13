import fastify from 'fastify';
import { FastifyInstance } from 'fastify';
import { socketsServer } from '.';
import { getTwitchAppClientID } from './misc';

export class Server
{
    constructor()
    {
        this.Instance = fastify();
    }

    listen(): void
    {
        if (this.Instance)
        {
            this.Instance.listen(41503, 'localhost', (err, address) =>
            {
                if (err)
                {
                    console.log('[Server] Server start error: ', err);
                }
                else
                {
                    console.log(`[Server] Server listening at ${address}`);
                }
            });
        }
    }

    setRoutes(): void
    {
        if (this.Instance)
        {
            this.Instance.get('/twitch-login', (request, reply) =>
            {
                reply.redirect(`https://id.twitch.tv/oauth2/authorize?response_type=token&client_id=${getTwitchAppClientID()}&redirect_uri=http://localhost:41503/token&scope=chat:read`);
            });
            
            this.Instance.get('/token', (request, reply) =>
            {
                reply.status(200).header('content-type', 'text/html; charset=utf-8').send('<!DOCTYPE HTML><html><head><title>Chatters</title></head><body>Вход выполнен успешно<script type="text/javascript">const hash=window.location.hash;console.log(hash);window.location.hash="";if(hash.startsWith("#access_token=")) { const tokenStr=hash.slice(14, 44); fetch(`http://${window.location.host}/sendToken?token=${tokenStr}`, { method: "POST" } )}</script></body></html>');
            });

            this.Instance.post('/sendToken', (request, reply) =>
            {
                const queryParams = request.query as Record<string, string>;

                if (queryParams && queryParams['token'])
                {
                    if (socketsServer)
                    {
                        socketsServer.sendNewTwitchToken(queryParams['token']);
                    }
                }

                reply.status(200).send("");
            });
        }
    }

    Instance: FastifyInstance | null = null;
}
