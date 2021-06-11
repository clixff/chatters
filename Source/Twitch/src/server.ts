import fastify from 'fastify';
import { FastifyInstance } from 'fastify';

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
                /** TOOO: Update URL */
                reply.redirect('https://twitch.tv');
            });
        }
    }

    Instance: FastifyInstance | null = null;
}
