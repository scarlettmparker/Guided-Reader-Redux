--
-- PostgreSQL database dump
--

-- Dumped from database version 17.2
-- Dumped by pg_dump version 17.2 (Ubuntu 17.2-1.pgdg22.04+1)

SET statement_timeout = 0;
SET lock_timeout = 0;
SET idle_in_transaction_session_timeout = 0;
SET transaction_timeout = 0;
SET client_encoding = 'UTF8';
SET standard_conforming_strings = on;
SELECT pg_catalog.set_config('search_path', '', false);
SET check_function_bodies = false;
SET xmloption = content;
SET client_min_messages = warning;
SET row_security = off;

--
-- Name: InteractionType; Type: TYPE; Schema: public; Owner: -
--

CREATE TYPE public."InteractionType" AS ENUM (
    'LIKE',
    'DISLIKE'
);


SET default_tablespace = '';

SET default_table_access_method = heap;

--
-- Name: Annotation; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public."Annotation" (
    id integer NOT NULL,
    start integer NOT NULL,
    "end" integer NOT NULL,
    description text NOT NULL,
    text_id integer NOT NULL,
    created_at integer NOT NULL,
    user_id integer NOT NULL
);


--
-- Name: Annotation_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public."Annotation_id_seq"
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: Annotation_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public."Annotation_id_seq" OWNED BY public."Annotation".id;


--
-- Name: Audio; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public."Audio" (
    id integer NOT NULL,
    audio_file text NOT NULL,
    vtt_file text NOT NULL,
    submission_group text NOT NULL,
    submission_url text
);


--
-- Name: Audio_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public."Audio_id_seq"
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: Audio_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public."Audio_id_seq" OWNED BY public."Audio".id;


--
-- Name: Text; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public."Text" (
    id integer NOT NULL,
    text text NOT NULL,
    language text NOT NULL,
    text_object_id integer NOT NULL,
    audio_id integer,
    author_id integer
);


--
-- Name: TextGroup; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public."TextGroup" (
    id integer NOT NULL,
    group_name text NOT NULL,
    group_url text
);


--
-- Name: TextGroup_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public."TextGroup_id_seq"
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: TextGroup_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public."TextGroup_id_seq" OWNED BY public."TextGroup".id;


--
-- Name: TextObject; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public."TextObject" (
    id integer NOT NULL,
    title text NOT NULL,
    level text NOT NULL,
    group_id integer,
    brief text
);


--
-- Name: TextObject_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public."TextObject_id_seq"
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: TextObject_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public."TextObject_id_seq" OWNED BY public."TextObject".id;


--
-- Name: Text_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public."Text_id_seq"
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: Text_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public."Text_id_seq" OWNED BY public."Text".id;


--
-- Name: User; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public."User" (
    id integer NOT NULL,
    username text NOT NULL,
    levels text[],
    discord_id text DEFAULT '-1'::integer,
    account_creation_date integer NOT NULL,
    avatar text DEFAULT '-1'::text NOT NULL,
    nickname text DEFAULT 'None'::text NOT NULL,
    accepted_policy boolean DEFAULT false NOT NULL,
    password text,
    discord_status boolean DEFAULT false,
    email text
);


--
-- Name: UserAnnotationInteraction; Type: TABLE; Schema: public; Owner: -
--

CREATE TABLE public."UserAnnotationInteraction" (
    id integer NOT NULL,
    user_id integer NOT NULL,
    annotation_id integer NOT NULL,
    type public."InteractionType" NOT NULL
);


--
-- Name: UserAnnotationInteraction_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public."UserAnnotationInteraction_id_seq"
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: UserAnnotationInteraction_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public."UserAnnotationInteraction_id_seq" OWNED BY public."UserAnnotationInteraction".id;


--
-- Name: User_id_seq; Type: SEQUENCE; Schema: public; Owner: -
--

CREATE SEQUENCE public."User_id_seq"
    AS integer
    START WITH 1
    INCREMENT BY 1
    NO MINVALUE
    NO MAXVALUE
    CACHE 1;


--
-- Name: User_id_seq; Type: SEQUENCE OWNED BY; Schema: public; Owner: -
--

ALTER SEQUENCE public."User_id_seq" OWNED BY public."User".id;


--
-- Name: mv_textobject_list; Type: MATERIALIZED VIEW; Schema: public; Owner: -
--

CREATE MATERIALIZED VIEW public.mv_textobject_list AS
 SELECT id,
    title,
    level,
    group_id
   FROM public."TextObject"
  ORDER BY id
  WITH NO DATA;


--
-- Name: Annotation id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public."Annotation" ALTER COLUMN id SET DEFAULT nextval('public."Annotation_id_seq"'::regclass);


--
-- Name: Audio id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public."Audio" ALTER COLUMN id SET DEFAULT nextval('public."Audio_id_seq"'::regclass);


--
-- Name: Text id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public."Text" ALTER COLUMN id SET DEFAULT nextval('public."Text_id_seq"'::regclass);


--
-- Name: TextGroup id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public."TextGroup" ALTER COLUMN id SET DEFAULT nextval('public."TextGroup_id_seq"'::regclass);


--
-- Name: TextObject id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public."TextObject" ALTER COLUMN id SET DEFAULT nextval('public."TextObject_id_seq"'::regclass);


--
-- Name: User id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public."User" ALTER COLUMN id SET DEFAULT nextval('public."User_id_seq"'::regclass);


--
-- Name: UserAnnotationInteraction id; Type: DEFAULT; Schema: public; Owner: -
--

ALTER TABLE ONLY public."UserAnnotationInteraction" ALTER COLUMN id SET DEFAULT nextval('public."UserAnnotationInteraction_id_seq"'::regclass);


--
-- Name: Annotation Annotation_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public."Annotation"
    ADD CONSTRAINT "Annotation_pkey" PRIMARY KEY (id);


--
-- Name: Audio Audio_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public."Audio"
    ADD CONSTRAINT "Audio_pkey" PRIMARY KEY (id);


--
-- Name: TextGroup TextGroup_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public."TextGroup"
    ADD CONSTRAINT "TextGroup_pkey" PRIMARY KEY (id);


--
-- Name: TextObject TextObject_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public."TextObject"
    ADD CONSTRAINT "TextObject_pkey" PRIMARY KEY (id);


--
-- Name: Text Text_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public."Text"
    ADD CONSTRAINT "Text_pkey" PRIMARY KEY (id);


--
-- Name: UserAnnotationInteraction UserAnnotationInteraction_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public."UserAnnotationInteraction"
    ADD CONSTRAINT "UserAnnotationInteraction_pkey" PRIMARY KEY (id);


--
-- Name: User User_pkey; Type: CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public."User"
    ADD CONSTRAINT "User_pkey" PRIMARY KEY (id);


--
-- Name: Text_audioId_key; Type: INDEX; Schema: public; Owner: -
--

CREATE UNIQUE INDEX "Text_audioId_key" ON public."Text" USING btree (audio_id);


--
-- Name: UserAnnotationInteraction_userId_annotationId_key; Type: INDEX; Schema: public; Owner: -
--

CREATE UNIQUE INDEX "UserAnnotationInteraction_userId_annotationId_key" ON public."UserAnnotationInteraction" USING btree (user_id, annotation_id);


--
-- Name: idx_annotation_composite; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX idx_annotation_composite ON public."Annotation" USING btree (text_id, start, "end");


--
-- Name: idx_annotation_text_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX idx_annotation_text_id ON public."Annotation" USING btree (text_id);


--
-- Name: idx_annotation_user_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX idx_annotation_user_id ON public."Annotation" USING btree (user_id);


--
-- Name: idx_textobject_composite; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX idx_textobject_composite ON public."TextObject" USING btree (id, title, level, group_id);


--
-- Name: idx_textobject_level; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX idx_textobject_level ON public."TextObject" USING btree (level);


--
-- Name: idx_textobject_title; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX idx_textobject_title ON public."TextObject" USING btree (title);


--
-- Name: idx_user_annotation_interaction_user_id; Type: INDEX; Schema: public; Owner: -
--

CREATE INDEX idx_user_annotation_interaction_user_id ON public."UserAnnotationInteraction" USING btree (user_id, type);


--
-- Name: mv_textobject_list_id_idx; Type: INDEX; Schema: public; Owner: -
--

CREATE UNIQUE INDEX mv_textobject_list_id_idx ON public.mv_textobject_list USING btree (id);


--
-- Name: Text Text_audioId_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public."Text"
    ADD CONSTRAINT "Text_audioId_fkey" FOREIGN KEY (audio_id) REFERENCES public."Audio"(id) ON UPDATE CASCADE ON DELETE SET NULL;


--
-- Name: Text Text_authorId_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public."Text"
    ADD CONSTRAINT "Text_authorId_fkey" FOREIGN KEY (author_id) REFERENCES public."User"(id) ON DELETE CASCADE;


--
-- Name: Text Text_textObjectId_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public."Text"
    ADD CONSTRAINT "Text_textObjectId_fkey" FOREIGN KEY (text_object_id) REFERENCES public."TextObject"(id) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- Name: UserAnnotationInteraction UserAnnotationInteraction_annotationId_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public."UserAnnotationInteraction"
    ADD CONSTRAINT "UserAnnotationInteraction_annotationId_fkey" FOREIGN KEY (annotation_id) REFERENCES public."Annotation"(id) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- Name: UserAnnotationInteraction UserAnnotationInteraction_userId_fkey; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public."UserAnnotationInteraction"
    ADD CONSTRAINT "UserAnnotationInteraction_userId_fkey" FOREIGN KEY (user_id) REFERENCES public."User"(id) ON UPDATE CASCADE ON DELETE RESTRICT;


--
-- Name: TextObject group_id; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public."TextObject"
    ADD CONSTRAINT group_id FOREIGN KEY (group_id) REFERENCES public."TextGroup"(id) NOT VALID;


--
-- Name: Annotation text_id; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public."Annotation"
    ADD CONSTRAINT text_id FOREIGN KEY (text_id) REFERENCES public."Text"(id) NOT VALID;


--
-- Name: Annotation user_id; Type: FK CONSTRAINT; Schema: public; Owner: -
--

ALTER TABLE ONLY public."Annotation"
    ADD CONSTRAINT user_id FOREIGN KEY (user_id) REFERENCES public."User"(id) NOT VALID;


--
-- PostgreSQL database dump complete
--

